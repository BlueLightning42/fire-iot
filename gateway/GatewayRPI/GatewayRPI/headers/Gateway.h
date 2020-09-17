#pragma once

#include "GatewayRPI.h"

#include "Gateway.h"
#include "Storage.h"
#include "Communication.h"


class Gateway {
 public:
	Gateway();
	~Gateway();
 private:
	std::vector<Device> tracked_devices;
	std::vector<Message> messages;
	bool running;
	bool message_recived;
	void mainLoop();
	void pollMessages();
	void updateTrackedDevices();
	void checkForTimeouts();
};
