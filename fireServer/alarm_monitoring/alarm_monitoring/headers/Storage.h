#pragma once

#include "../monitoring.h"
#include "Communication.h"
/*
stores device id.
also include state of device? type of device?
*/
struct Device {
	uint16_t id;
	std::string name;

	std::chrono::steady_clock::time_point last_communication;
	std::chrono::steady_clock::time_point first_detection;
	const bool operator<(const uint16_t& _id) const {
		return this->id < _id;
	}
	const bool operator<(const Device& other) const {
		return this->id < other.id;
	}
	const bool operator==(const std::string _name) const {
		return !this->name.compare(_name);
	}
	Device(): id(0), name(""),
		last_communication(std::chrono::steady_clock::time_point::max()),
		first_detection(std::chrono::steady_clock::time_point::max()) {}
	Device(uint16_t _id, std::string _name): id(_id), name(_name),
		last_communication(std::chrono::steady_clock::time_point::max()),
		first_detection(std::chrono::steady_clock::time_point::max()) {}
};

/*
retrives device data and stores it in a string. (small overhead)
*/
std::string prepareAlert(uint16_t, typ::Type);
