#include "../headers/Storage.h"


std::vector<Device> loadDevices() {
	const char* filename = "stored_devices.db";
	if (/*file open == */ true ) {
		log(logging::info, "sucessfully read data from {}", filename);
	} else {
		log(logging::critical, "unable to read data from {}.\n", filename);
		//runtime error?
	}
	return {};
}

/*
on shutdown store all devices into a small database file (sqllite3?)
*/
void storeDevices(std::vector<Device> data) {

}