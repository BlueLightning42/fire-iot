#pragma once

#include "../GatewayRPI.h"

#include "Storage.h"
#include "Communication.h"

inline constexpr const char* config_file_name = "myConfig.txt";

class Gateway {
 public:
	 Gateway();
	~Gateway();
 private:
	void readConfig();
	void sendAlert(const std::string& msg);
	std::filesystem::file_time_type last_config_update;
	std::chrono::steady_clock::time_point last_files_updated; // to check for updates/reset every few minutes.
	std::chrono::steady_clock::time_point last_logging_updated; // to reset logger every few hours.

	std::vector<Device> tracked_devices; // vector of devices that are communicating with the Gateway
	std::vector<Message> messages; // vector acting as a stack of all recived messages.
	std::vector<Message> recived; // vector acting as a stack of all recived messages for the message thread
	std::mutex m;
	bool message_recived;
	std::thread message_thread;

	// config info.
	std::string host_name; // for alert connection
	std::string client_name; // for alert connection
	std::string username; // for alert connection
	std::string password; // for alert connection
	std::string topic; // for alert connection
	std::chrono::milliseconds timeout_no_communication;
	std::chrono::milliseconds timeout_alarm_blaring;

	// mainloop of the program
	void mainLoop();

	void makeMessageThread();

	void pollMessages();
	void periodicReset();
	void updateTrackedDevices();
	void checkForTimeouts();
};
