#include "../headers/Gateway.h"

// the main logic of the program. 

// the following section is just setting up Ctrl+C as a method of closing the program.
#ifdef __linux__
#include <signal.h>

//Flag for Ctrl-C
namespace program{
	volatile sig_atomic_t running = true;
}

void sig_handler(int sig){
	log(logging::warn, "Break received, exiting!"); // probably not thread safe going remove in the future but for now leaving it.
  	program::running = false;	
}
void setup_CTRLC() {
	signal(SIGINT, sig_handler); // Ctrl-C
}

#else
#ifndef NOMINMAX
#define NOMINMAX //windows.h is a awful mess that defines gross coliding macros...that is all
#endif
#include <windows.h>
//Flag for Ctrl-C on windows
namespace program {
	volatile bool running = true;
}

BOOL WINAPI consoleHandler(DWORD signal) {
	if (signal == CTRL_C_EVENT) {
		log(logging::warn, "Break received, exiting!");
		program::running = false;
	}

	return TRUE;
}

void setup_CTRLC() {
	if (!SetConsoleCtrlHandler(consoleHandler, TRUE))
		log(logging::critical, "Unable to setup Ctrl+C handler");
}
#endif

// the follwing is simply setting up and cleaning up the program state
Gateway::Gateway(): message_recived(false) {
	setup_CTRLC();
	openLogger(); // HAS to be opened before any logging can be done otherwise will crash on file write
	readConfig(); // Store some configurable values. (can be changed by users)
	tracked_devices = loadDevices(); // gets a sorted vector of all the id's of stored devices.
	initLoRa();
	makeMessageThread();
	auto now = std::chrono::steady_clock::now();
	last_files_updated = now;
	last_logging_updated = now;
	mainLoop();
}
Gateway::~Gateway() {
	// storeDevices(tracked_devices); // store last state of devices.
	closeLoRa();
	closeLogger(); // Good manners to close logging resources...the operating system probably can if its shutoff but its good manners all the same.
}

inline void Gateway::sendAlert(const std::string& msg) {
	/* Replace send_message with a custom function or library populated with relevant info if changed from MQTT
	*/
	send_message(msg.c_str(), host_name.c_str(), client_name.c_str(), username.c_str(), password.c_str(), topic.c_str());
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
	while ( program::running ) {
		pollMessages();
		if ( !messages.empty() ) {
			updateTrackedDevices();
		}
		checkForTimeouts();
		periodicReset();
	}
}

void Gateway::periodicReset() {
	// every minute or so check if files have changed.
	using namespace std::chrono;
	namespace fs = std::filesystem;
	auto now = steady_clock::now();
	if (now - this->last_files_updated > 2min) {
		this->last_files_updated = now;
		if (fs::exists(config_file_name)) {
			auto file = fs::path(config_file_name);
			auto last_write = fs::last_write_time(file);
			if (last_write != this->last_config_update) {
				log(logging::warn, "Config file was changed...attempting to re-read it.");
				readConfig();
			}
		}
		else { // if file doesn't exist
			log(logging::warn, "Config file not found...attempting to reset it.");
			readConfig();
		}
	}
	// every 24 hours or so reset logger...only comes into effect if this becomes a long running application
	if (now - this->last_logging_updated > 24h) {
		log(logging::info, "Refreshing logger");
		this->last_logging_updated = now;
		closeLogger();
		openLogger();
	}
}
void Gateway::updateTrackedDevices() {
	/* Itterate over all recived messages.
	 * If the message is a heartbeat update its last_communication so it doesn't get stale and 
	 * If the message is a Alarm start a countdown/ check the current countdown for sending a alarm message
	*/
	if (tracked_devices.empty()) {
		log(logging::critical, "empty tracked devices");
		messages.clear();
		return;
	}
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
			if (to_update == tracked_devices.end()) {
				log(logging::warn, "heartbeat recived from unknown device with id: {}", message.id);
			}else if ( to_update->id == message.id ) {
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
			if (dev == tracked_devices.end()) {
				log(logging::critical, "alarm message recived from unknown device with id: {}", message.id);
			}else if (dev->id == message.id) {
				if ( dev->first_detection == steady_clock::time_point::max() ) {
					// if first_detection is at sentinal value set it to now/ start counting down.
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
				// make timout time equal to alarm time to make sure the ->alarm recived ->no communicaiton case happens much lower than the 3 minutes.
				dev->last_communication = now - timeout_alarm_blaring + timeout_no_communication;
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
	using namespace std::chrono;
	auto now = steady_clock::now();
	constexpr auto max_time = steady_clock::time_point::max();
	for ( auto &device : tracked_devices ) {
		if ( now - device.last_communication > timeout_no_communication ) {
			std::string fire_alert;
			if (device.first_detection != max_time) {
				fire_alert = prepareAlert(device.id, typ::no_communication_and_fire);	
			}else {
				fire_alert = prepareAlert(device.id, typ::no_communication);
			}
			this->sendAlert(fire_alert);
			device.last_communication = max_time; //forces it to stop giving updates after one tick
			device.first_detection = max_time;
		}
	}
}