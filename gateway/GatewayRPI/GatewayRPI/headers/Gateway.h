#pragma once

#include "../GatewayRPI.h"

#include "Gateway.h"
#include "Storage.h"
#include "Communication.h"


class Gateway {
 public:
	Gateway();
	~Gateway();
 private:
	std::vector<Device> tracked_devices; // vector of devices that are communicating with the Gateway
	std::vector<Message> messages; // vector acting as a stack of all recived messages.
	bool message_recived; // overlaps with !messages.empty() ... keep for readability or replace?

	std::string host_name; // for alert connection
	std::chrono::milliseconds timeout_no_communication;
	std::chrono::milliseconds timeout_alarm_blaring;

	bool running;
	void mainLoop();

	void pollMessages();
	void updateTrackedDevices();
	void checkForTimeouts();
};
