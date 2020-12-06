#pragma once

#include "../monitoring.h"

#include "Storage.h"
#include "Communication.h"

inline constexpr const char* config_file_name = "myConfig.txt";

class Monitor {
 public:
	 Monitor();
	~Monitor();
 private:
	void readConfig(int try_again=0);
	void loadDevices();
	void refreshDevicesInPlace();
	void sendAlert(const std::string& msg);
	std::filesystem::file_time_type last_config_update;         // stored last config update for reseting config with changes.
	std::filesystem::file_time_type last_database_update;		// stored database update for reloading tracked devices.
	std::chrono::steady_clock::time_point last_files_updated;   // to check for updates/reset every few minutes.
	std::chrono::steady_clock::time_point last_logging_updated; // to reset logger every few hours.

	std::vector<Device> tracked_devices; // vector of devices that are communicating with the Monitor
	std::vector<Message> messages; 		 // vector acting as a stack of all recived messages.
	std::vector<Message> recived; 		 // vector acting as a stack of all recived messages for the message thread
	std::mutex m;						 // mutex for syncing recived messages from reciver makeMessageSenderThread

	void initMQTT();

	// for output
	bool MQTT_connected;
	mqtt::async_client_ptr cli;
	std::string host_name,client_name,username,password,topic_name;
	void makeMessageSenderThread();

	// for input
	mqtt::async_client_ptr ttn_sub;
	std::string ttn_host,ttnClientName,AppID,AppKey;
	std::unique_ptr<ttn_connection_callback> ttn_cbk;
	void makeMessageReciverThread();

	void send_MQTT_message(const std::string& msg);

	void closeMQTT();

	// mainloop of the program
	std::chrono::milliseconds timeout_no_communication,timeout_alarm_blaring;
	void mainLoop();
	void pollMessages();
	void periodicReset();
	void checkDatabaseUpdate();
	void updateTrackedDevices();
	void checkForTimeouts();
};
