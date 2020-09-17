#pragma once

#include "GatewayRPI.h"

/*
stores device id. 
also include state of device? type of device?
*/
struct Device {
	uint16_t id;
	std::chrono::steady_clock::time_point last_communication;
	const bool operator<(const Device& rhs) const{
		return this->id < rhs.id;
	}
	const bool operator<(const uint16_t& _id) const {
		return this->id < _id;
	}
};

/*
on init/restart load all devices from a small database file (sqllite3?)
*/
std::vector<std::pair<Device, int>> loadDevices() {
	return {};
}

/*
on shutdown store all devices into a small database file (sqllite3?)
*/
void storeDevices(std::vector<std::pair<Device, int>> data) {

}
