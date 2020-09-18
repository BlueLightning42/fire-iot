#pragma once

#include "../GatewayRPI.h"

/*
stores device id. 
also include state of device? type of device?
*/
struct Device {
	uint16_t id;
	std::chrono::steady_clock::time_point last_communication;
	const bool operator<(const uint16_t& _id) const {
		return this->id < _id;
	}
	Device(): id(NULL), 
		last_communication(std::chrono::steady_clock::time_point::max()) {}
	Device(uint16_t _id): id(_id),
		last_communication(std::chrono::steady_clock::time_point::max()) {}
};

/*
on init/restart load all devices from a small database file (sqllite3?)
*/
std::vector<Device> loadDevices();

/*
on shutdown store all devices into a small database file (sqllite3?)
*/
void storeDevices(std::vector<Device> data);
