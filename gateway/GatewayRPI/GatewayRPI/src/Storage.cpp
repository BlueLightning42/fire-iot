#include "../headers/Storage.h"


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