#include "../headers/Storage.h"


const char* filename = "stored_devices.db";

std::vector<Device> loadDevices() {
	sqlite3* db;
	// open database connection
	int err = sqlite3_open(filename, &db);
	if ( err ) {
		log(logging::warn, "Can't open device database: {}", sqlite3_errmsg(db));
		sqlite3_close(db);
		return {};
	}
	// return all device id's from database
	std::vector<Device> returned_devices;
	sqlite3_stmt *stmt = 0;
	sqlite3_prepare_v2(db, "SELECT id FROM StoredDevices", -1, &stmt, 0);

	while ( sqlite3_step(stmt) == SQLITE_ROW ) { // While query has result-rows.
		for ( int colIndex = 0; colIndex < sqlite3_column_count(stmt); colIndex++ ) {
			int result = sqlite3_column_int(stmt, colIndex);
			returned_devices.emplace_back((uint16_t)result);
		}
	}
	sqlite3_finalize(stmt);
	sqlite3_close(db);
	log(logging::info, "Read database and returned {} tracked devices.", returned_devices.size());

	return returned_devices;
}

/*
on shutdown store all devices into a small database file (sqllite3?)
*/
void storeDevices(std::vector<Device> data) {

}

std::string prepareAlert(uint16_t id, typ::Type alert_type) {
	sqlite3* db;
	// open database connection
	int err = sqlite3_open(filename, &db);
	if ( err ) {
		log(logging::warn, "Can't open device database: {}", sqlite3_errmsg(db));
		sqlite3_close(db);
		return {};
	}
	//untested yet
	sqlite3_stmt* stmt = 0;
	sqlite3_prepare_v2(db, "SELECT address, postal_code FROM StoredDevices WHERE id = ?1", -1, &stmt, 0);
	err = sqlite3_bind_int(stmt, 1, id);
	if ( err ) {
		log(logging::warn, "Can't bind id in getdevice info {}", sqlite3_errmsg(db));
	}

	const char* alert_name = "General Error";
	switch ( alert_type ) {
	 case typ::alarm: alert_name = "Alarm is On for too long";
	 case typ::no_communication: alert_name = "No communication recived for some time";
	 default: log(logging::critical, "Unknown alert_type: {} passed to prepareAlert function", alert_type);
	}
	std::string alert = fmt::format("Warning: '{}' recived from {},{}", alert_name, 0, 0);

	sqlite3_finalize(stmt);
	sqlite3_close(db);
}