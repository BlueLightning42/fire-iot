#pragma once

#include "../GatewayRPI.h"

#include "Storage.h"
#include "Communication.h"


class Gateway {
 public:
	 Gateway();
	~Gateway();
	void sendAlert(const std::string& msg);
 private:
	void readConfig();
	std::vector<Device> tracked_devices; // vector of devices that are communicating with the Gateway
	std::vector<Message> messages; // vector acting as a stack of all recived messages.
	bool message_recived; // overlaps with !messages.empty() ... keep for readability or replace?

	std::string host_name; // for alert connection
	std::string client_name; // for alert connection
	std::string username; // for alert connection
	std::string password; // for alert connection
	std::string topic; // for alert connection
	std::chrono::milliseconds timeout_no_communication;
	std::chrono::milliseconds timeout_alarm_blaring;

	// mainloop of the program
	bool running;
	void mainLoop();

	void pollMessages();
	void updateTrackedDevices();
	void checkForTimeouts();
};
