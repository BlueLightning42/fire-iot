#include "../headers/Storage.h"
#include "../headers/MonitorApp.h"

static const char* database_name = "/var/lib/fireiot/stored_devices.db";

static std::filesystem::file_time_type getLastWriteTime(const char* filename){
	namespace fs = std::filesystem;
	auto file = fs::path(filename);
	auto last_write = fs::last_write_time(file);
}

void Monitor::loadDevices() {
	try {
		SQLite::Database db(database_name, SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE | SQLite::OPEN_FULLMUTEX, 200);
		db.exec(R"(CREATE TABLE IF NOT EXISTS StoredDevices (
			id INTEGER PRIMARY KEY,
			dev_name  VARCHAR(25) UNIQUE,
			address  VARCHAR(25) NOT NULL,
			postal_code VARCHAR(20),
			device_type VARCHAR(20)
		) )");

		std::vector<Device> returned_devices;
		SQLite::Statement query(db, "SELECT id,dev_name FROM StoredDevices ORDER BY id");
		while (query.executeStep()){
			uint16_t id = query.getColumn(0);
			std::string dev_name = query.getColumn(1);
			returned_devices.emplace_back(id, dev_name);
		}
		if (returned_devices.size() == 0){
			log(logging::warn, "Read database and found 0 devices...adding dummy null device");
			db.exec(R"(INSERT INTO StoredDevices VALUES (0, "NOT_A_DEVICE",  "-1 null drive", "X0X123", "microwave") )");
			returned_devices.emplace_back(0, "NOT_A_DEVICE");
#ifdef __linux__
			namespace fs = std::filesystem;
			std::error_code ec;
			fs::permissions(database_name, fs::perms::owner_all | fs::perms::group_all, fs::perm_options::add, ec); // make sure database can be acessed by fire_iot group
#endif
		}else{
			log(logging::info, "Read database and returned {} tracked devices.", returned_devices.size());
		}
		std::sort(returned_devices.begin(), returned_devices.end()); // should already be sorted but just in case.
		tracked_devices = std::move(returned_devices);
	}catch (std::exception& e)	{
		log(logging::critical, "SQLite exception: {}\ncouldn't load Devices\n", e.what());
	}
	this->last_database_update = getLastWriteTime(config_file_name);
}
void Monitor::refreshDevicesInPlace(){
	try {
		SQLite::Database db(database_name, SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE | SQLite::OPEN_FULLMUTEX, 200);
		SQLite::Statement query(db, "SELECT id,dev_name FROM StoredDevices ORDER BY id");

		std::vector<Device> new_devices;
		auto tracked_itter = tracked_devices.begin();
		while (query.executeStep()){
			uint16_t querry_id = query.getColumn(0);
			std::string dev_name = query.getColumn(1);
			if (tracked_itter != tracked_devices.end()){
				if (tracked_itter->id == querry_id){
					tracked_itter->name = dev_name;
				}else{
					bool found = false;
					auto searched = std::lower_bound(tracked_devices.begin(), tracked_devices.end(), querry_id);
					if( searched != tracked_devices.end()){
						if( searched->id == querry_id){
							tracked_itter = searched;
							tracked_itter->name = dev_name;
							found = true;
						}
					}
					if (!found){
						new_devices.emplace_back(querry_id, dev_name);
					}
				}
				tracked_itter++;
			}
		}
		tracked_devices.insert(tracked_devices.end(), new_devices.begin(), new_devices.end());
		std::sort(tracked_devices.begin(), tracked_devices.end());
		log(logging::info, "reread database- now tracking {} devices.", tracked_devices.size());
	}catch (std::exception& e)	{
		log(logging::critical, "SQLite exception: {}\ncouldn't load Devices\n", e.what());
	}
	this->last_database_update = getLastWriteTime(config_file_name);
}

void Monitor::checkDatabaseUpdate(){
	namespace fs = std::filesystem;
	if (fs::exists(database_name)) {
		auto file = fs::path(database_name);
		auto last_write = fs::last_write_time(file);
		if (last_write != this->last_database_update) {
			log(logging::info, "{} has been added to- refreshing tracked devices.", database_name);
			refreshDevicesInPlace();
		}
		fmt::print("last right time is the same");
	}else{
		log(logging::critical, "database not found...resetting application by reloading devices from scratch.");
		loadDevices(); //load from scratch
		log(logging::info, "the database file should really never be deleted. Check to see if something happend.");
	}
}
void Monitor::periodicReset() {
	// every minute or so check if files have changed.
	using namespace std::chrono;
	namespace fs = std::filesystem;
	auto now = steady_clock::now();
	if (now - this->last_files_updated > 2min) {
		this->last_files_updated = now;
		if (fs::exists(config_file_name)) {
			auto file = fs::path(config_file_name);
			auto last_write = fs::last_write_time(file);
			if (last_write != this->last_config_update) {
				log(logging::warn, "Config file was changed...attempting to re-read it.");
				readConfig();
			}
		}
		else { // if file doesn't exist
			log(logging::warn, "Config file not found...attempting to reset it.");
			readConfig();
		}
		checkDatabaseUpdate();
	}
	// every 24 hours or so reset logger...only comes into effect if this becomes a long running application
	if (now - this->last_logging_updated > 24h) {
		log(logging::info, "Refreshing logger");
		this->last_logging_updated = now;
		closeLogger();
		openLogger();
	}
}


std::string prepareAlert(uint16_t id, typ::Type alert_type) {
	const char* alert_name = "General Error";
	switch (alert_type) {
	 case typ::alarm: alert_name = "Alarm is On for too long"; break;
	 case typ::no_communication: alert_name = "No communication recived for some time"; break;
	 case typ::no_communication_and_fire: alert_name = "No communication recived for some time and last message sent was alarm!"; break;
	 case typ::error: alert_name = "General Error recived"; break;
	 default: log(logging::critical, "Unknown alert_type: {} passed to prepareAlert function", alert_type);
	}

	using namespace std::chrono;
	auto now = system_clock::to_time_t(system_clock::now());

	try {
		SQLite::Database db(database_name, SQLite::OPEN_READONLY | SQLite::OPEN_FULLMUTEX, 200);
		std::string alert;

		SQLite::Statement query(db, "SELECT address, postal_code FROM StoredDevices WHERE id = ?1");
		query.bind(1, id);
		while (query.executeStep()) {
			alert = fmt::format("Warning: '{}'\nRecived at: [{:%Y-%m-%d %H:%M:%S}]\nFrom {}, {}\n",
								alert_name, fmt::localtime(now), query.getColumn(0), query.getColumn(1));
		}
		return alert;
	}
	catch (std::exception& e) {
		log(logging::critical, "SQLite exception: {}\ncouldn't prepare alert for id: {}\n", e.what(), id);
	}
	return "Internal error preparing alert.";
}

// code for config file...basically a super limited version of the .ini format.
// moved to here to group all file related stuff in the same section.
static const char* default_config_text = \
R"(# Config file (can be changed)

# alert message
hostname = localhost:1883
ClientName = OG_Monitor
# please change these from defaults after setting up
username = fire
password = iot
topic = fire/alert

# ttn values for mqtt
ttn_host = us-west.thethings.network:1883
ttnClientName = OG_Monitor
# These values can be found in TTN after creating an application
AppID = XXX-XXX-XXX
AppKey = ttn-account-v2.XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX

# Timeout values are in milliseconds
timeout_no_communication = 180000
timeout_alarm_blaring = 20000
)";

// I've already added too many dependancies...adding hjson or ini or something just to make a config file that people won't touch often is too much
// although the more settings I add the closer I am to replacing it
void Monitor::readConfig(int try_again) {
	// its just a config file...overhead of this is neligible and optomizations are a waste of time...considering its only read at initialization (and file deletion/editing)
	// tfw it may seem weird to use c++ streams for input and fmt files for output but the streams library is annoying me lately...I wish fmt handled input too
	std::ifstream config_file(config_file_name);
	if ( config_file.is_open() ) {
		std::string line;
		while ( std::getline(config_file, line) ) {
			line.erase(std::remove_if(line.begin(), line.end(), isspace), line.end()); // remove whitespace
			if ( line[0] == '#' || line.empty() ) continue; //ignore comment lines and empty lines

			auto delimiterPos = line.find("=");
			auto name = line.substr(0, delimiterPos);
			auto value = line.substr(delimiterPos + 1);

			// could be prettyfied with Pattern Matching if that decides to work its way into c++23 but w/e...if I cared more I'd swap to a dedicated library.
			if      ( name == "hostname" )                   host_name = value;
			else if ( name == "ClientName" )                 client_name = value;
			else if ( name == "username" )                   username = value;
			else if ( name == "password" )                   password = value;
			else if ( name == "topic" )                      topic_name = value;
			else if ( name == "timeout_no_communication" )   timeout_no_communication = std::chrono::milliseconds(std::stoi(value));
			else if ( name == "timeout_alarm_blaring" )      timeout_alarm_blaring = std::chrono::milliseconds(std::stoi(value));
			else if ( name == "ttn_host" )                   ttn_host = value;
			else if ( name == "ttnClientName" )              ttnClientName = value;
			else if ( name == "AppID" )                      AppID = value;
			else if ( name == "AppKey" )                     AppKey = value;

		}
		try_again+=10;
	} else {
		log(logging::warn, "Couldn't open config file.");

		namespace fs = std::filesystem;
		auto out = fmt::output_file(config_file_name);
		std::error_code ec;
		fs::permissions(config_file_name, fs::perms::all, fs::perm_options::add, ec); // make sure config file has all permissions... if rwxrwxrwx is bad...then change it to rwxrwx--- I guess

		out.print(default_config_text);
		log(logging::info, "Replacing config with defaults");

		//default values are placed-rerun config once more and it will try to read them after remaking them.
		try_again++;
	}
	if(try_again == 1) readConfig(try_again);
	last_config_update = getLastWriteTime(config_file_name);
}
