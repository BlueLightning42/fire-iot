#include "../headers/Storage.h"
#include "../headers/Gateway.h"

static const char* database_name = "stored_devices.db";

std::vector<Device> loadDevices() {
	try {
		SQLite::Database db(database_name, SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE | SQLite::OPEN_FULLMUTEX, 200);
		db.exec(R"(CREATE TABLE IF NOT EXISTS StoredDevices (
			id int PRIMARY KEY,
			address  VARCHAR(25) NOT NULL,
			postal_code VARCHAR(20),
			device_type VARCHAR(20)
		) )");
		//db.exec(R"(INSERT INTO StoredDevices VALUES (0, " -1 null drive", "X0X123", "microwave") )");
		std::vector<Device> returned_devices;
		SQLite::Statement query(db, "SELECT id FROM StoredDevices");
		while (query.executeStep()){
			int id = query.getColumn(0);
			returned_devices.emplace_back((uint16_t)id);
		}
		if (returned_devices.size() == 0)
			log(logging::warn, "Read database and found 0 devices.");
		else
			log(logging::info, "Read database and returned {} tracked devices.", returned_devices.size());
		return returned_devices;
	}catch (std::exception& e)	{
		log(logging::critical, "SQLite exception: {}\ncouldn't load Devices\n", e.what());
	}
	return {};
}

/*
on shutdown store all devices into a small database file (sqllite3?) ?
currently not implemented because devices are populated 
void storeDevices(std::vector<Device> data) {

}
*/

std::string prepareAlert(uint16_t id, typ::Type alert_type) {
	const char* alert_name = "General Error";
	switch (alert_type) {
	 case typ::alarm: alert_name = "Alarm is On for too long"; break;
	 case typ::no_communication: alert_name = "No communication recived for some time"; break;
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
		log(logging::info, "Read database and setup a alert");
		return alert;
	}
	catch (std::exception& e) {
		log(logging::critical, "SQLite exception: {}\ncouldn't prepare alert for id: {}\n", e.what(), id);
	}
	return "Internal error preparing alert.";
}

// code for config file...basically a super limited version of the .ini format.
// moved to here to group all file related stuff in the same section.
static const char* default_config_text = R"(# Config file (can be changed)

# alert message 
hostname = localhost
ClientName = OG_Gateway
username = fire
password = iot
topic = alert

# Timeout values are in milliseconds
timeout_no_communication = 180000
timeout_alarm_blaring = 20000
)";

// I've already added too many dependancies...adding hjson or ini or something just to make a config file that people won't touch often is too much
void Gateway::readConfig() {
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
			
			if ( name == "hostname" ) host_name = value;
			else if ( name == "ClientName" ) client_name = value;
			else if ( name == "username" ) username = value;
			else if ( name == "password" ) password = value;
			else if ( name == "topic" ) topic = value;
			else if ( name == "timeout_no_communication" ) timeout_no_communication = std::chrono::milliseconds(std::stoi(value));
			else if ( name == "timeout_alarm_blaring" ) timeout_alarm_blaring = std::chrono::milliseconds(std::stoi(value));
		}
	} else {
		log(logging::warn, "Couldn't open config file.");
		
		auto out = fmt::output_file(config_file_name);

		out.print(default_config_text);
		log(logging::info, "Replacing config with defaults");

		//default values
		host_name = "localhost";
		client_name = "OG_Gateway";
		username = "fire";
		password = "iot";
		topic = "alert";
		using namespace std::chrono;
		timeout_no_communication = 3min;
		timeout_alarm_blaring = 20s;
	}

	auto file = std::filesystem::path(config_file_name);
	last_config_update = std::filesystem::last_write_time(file);
}