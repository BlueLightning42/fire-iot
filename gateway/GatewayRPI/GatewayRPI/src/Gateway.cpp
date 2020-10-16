#include "../headers/Gateway.h"

static const char* default_config_text = R"(# Config file (can be changed)

# alert message 
hostname = localhost
ClientName = OG_Gateway
username = fire
password = iot

# Timeout values are in milliseconds
timeout_no_communication = 180000
timeout_alarm_blaring = 30000
)";

// I've already added too many dependancies...adding hjson or ini or something just to make a config file that people won't touch often is too much
void Gateway::readConfig() {
	// its just a config file...overhead of this is neligible and optomizations are a waste of time.
	// tfw it may seem weird to use c++ streams for input and c style files for output but the streams library is annoying me lately...I wish fmt handled input too
	std::ifstream config_file("myConfig.txt");
	if ( config_file.is_open() ) {
		std::string line;
		while ( std::getline(config_file, line) ) {
			line.erase(std::remove_if(line.begin(), line.end(), isspace), line.end()); // remove whitespace
			if ( line[0] == '#' || line.empty() ) continue; //ignore comment lines and empty lines

			auto delimiterPos = line.find("=");
			auto name = line.substr(0, delimiterPos);
			auto value = line.substr(delimiterPos + 1);
			
			if ( name == "hostname" ) host_name = value;
			else if ( name == "ClientName" ) client_name = value;
			else if ( name == "username" ) username = value;
			else if ( name == "password" ) password = value;
			else if ( name == "timeout_no_communication" ) timeout_no_communication = std::chrono::milliseconds(std::stoi(value));
			else if ( name == "timeout_alarm_blaring" ) timeout_alarm_blaring = std::chrono::milliseconds(std::stoi(value));
		}
	} else {
		log(logging::warn, "Couldn't open config file.");
		
		std::FILE* out_config;
		fopen_s(&out_config, "myConfig.txt", "w");
		if ( !out_config ) {
			log(logging::critical, "couldn't write a new config file...problem with permissions?");
		} else {
			log(logging::info, "Replacing config with defaults");
			fmt::print(out_config, default_config_text);
		}
		std::fclose(out_config);

		host_name = "localhost";
		timeout_no_communication = std::chrono::minutes{ 3 };
		timeout_no_communication = std::chrono::seconds{ 30 };
	}
}

Gateway::Gateway(): running(true) {
	openLogger(); // HAS to be opened before any logging can be done otherwise will crash on file write
	readConfig(); // Store some configurable values. (can be changed by users)
	tracked_devices = loadDevices(); // gets a sorted vector of all the id's of stored devices.
}
Gateway::~Gateway() {
	storeDevices(tracked_devices); // store last state of devices.
	closeLogger(); // Good manners to close logging resources 
}

inline void Gateway::sendAlert(const std::string& msg) {
	/* Replace send_message with a custom function or library populated with relevant info if changed from MQTT
	*/
	send_message(msg.c_str(), host_name.c_str(), client_name.c_str(), username.c_str(), password.c_str());
}


void Gateway::mainLoop() {
	/* Endless loop for continous program.
	 * check for new messages.
	 * if messages recived update all the devices.
	 * then check for any timeouts
	 * repeat.
	 * TODO: as errors can appear in long running programs add a long running timer
	 *		 and reset data if its been changed? ie reset config data and reset tracked_devices
	 *       if the database or config file have been changed
	*/
	while ( running ) {
		pollMessages();
		if ( !messages.empty() ) {
			updateTrackedDevices();
		}
		checkForTimeouts();
	}
}

inline void Gateway::pollMessages() {
	/* Add all LoRa messages recived to the messages vector
	 * TODO: ALL OF THIS
	 */
}


void Gateway::updateTrackedDevices() {
	/* Itterate over all recived messages.
	 * If the message is a heartbeat update its last_communication so it doesn't get stale and 
	 * If the message is a Alarm start a countdown/ check the current countdown for sending a alarm message
	*/
	using namespace std::chrono;
	auto now = steady_clock::now();
	for ( const auto& message : messages ) {
		switch ( message.type ) {
		 case typ::heartbeat:{
			 /* Retrive the tracked device associated with the id in the message
			  * Lower bound can be used because the tracked_devices vector is sorted.
			  * Update the tracked communication time with the current time.
			 */
			auto to_update = std::lower_bound(tracked_devices.begin(), tracked_devices.end(), message.id);
			if ( to_update->id == message.id ) {
				log(logging::info, "Recived Ok signal from {}", message.id);
				to_update->last_communication = now;
				to_update->first_detection = steady_clock::time_point::max();
			} else {
				log(logging::warn, "heartbeat recived from unknown device with id: {}", message.id);
			}
			break;
		 }
		 case typ::alarm: {
			 /* Retrive the tracked device associated with the id in the message
			  * Lower bound can be used because the tracked_devices vector is sorted.
			  * if it is not being tracked (time_point::max() is used as a sentinal if its not already tracked.)
			  * then set the first detection to now.
			  * if the current alarm has been going on longer than the timout time then send a alert.
			 */
			auto dev = std::lower_bound(tracked_devices.begin(), tracked_devices.end(), message.id);
			if ( dev->id == message.id ) {
				 if ( dev->first_detection == steady_clock::time_point::max() ) {
					 log(logging::warn, "alarm started going off with id:{}", message.id);
					 dev->first_detection = now;
				 }
				 if ( now - dev->first_detection > timeout_alarm_blaring ) {
					 auto fire_alert = prepareAlert(message.id, message.type);
					 this->sendAlert(fire_alert);
					 // send repeated every timout
					 // dev->first_detection = now;
					 // send once unstil its ben set to heartbeat and back.
					 dev->first_detection = steady_clock::time_point::max() - milliseconds{ 1 };
				 }
			} else {
				log(logging::critical, "alarm message recived from unknown device with id: {}", message.id);
			}
			 break;
		 }
		 case typ::error:
		 default:{ // all error types are sent with thire error id...need to extend it for any error code added
			auto fire_alert = prepareAlert(message.id, message.type);
			this->sendAlert(fire_alert);
			break;
		 }
		}
	}
	// after all messages are processed remove them.
	messages.clear();
}

void Gateway::checkForTimeouts() {
	/* Itterate over all tracked devices and check to see if thire time_passed 
	 * is greater than the defined timeout time in timeout_no_communication
	 * if it has timed out then prepare a alert message and send it to send_message
	*/
	auto now = std::chrono::steady_clock::now();
	for ( const auto &device : tracked_devices ) {
		if ( now - device.last_communication > timeout_no_communication ) {
			auto fire_alert = prepareAlert(device.id, typ::no_communication);
			this->sendAlert(fire_alert);
		}
	}
}