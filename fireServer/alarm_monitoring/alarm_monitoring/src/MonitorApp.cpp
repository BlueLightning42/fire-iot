#include "../headers/MonitorApp.h"

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
Monitor::Monitor(): MQTT_connected(false) {
	setup_CTRLC();
	openLogger(); // HAS to be opened before any logging can be done otherwise will crash on file write
	readConfig(); // Store some configurable values. (can be changed by users)
	tracked_devices = loadDevices(); // gets a sorted vector of all the id's of stored devices.
	makeMessageThread();
	auto now = std::chrono::steady_clock::now();
	last_files_updated = now;
	last_logging_updated = now;
	initMQTT();
	mainLoop();
}
Monitor::~Monitor() {
	// storeDevices(tracked_devices); // store last state of devices.
	closeMQTT();
	closeLogger(); // Good manners to close logging resources...the operating system probably can if its shutoff but its good manners all the same.
}

inline void Monitor::sendAlert(const std::string& msg) {
	// makes it easier to replace MQTT by swapping out this part.
	send_MQTT_message(msg);
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //
//                        Important application Logic below here
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ //

void Monitor::mainLoop() {
	/* Endless loop for continous program.
	 * check for new messages.
	 * if messages recived update all the devices.
	 * then check for any timeouts
	 * repeat.
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

void Monitor::periodicReset() {
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
void Monitor::updateTrackedDevices() {
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
		auto tracked = std::find(tracked_devices.begin(), tracked_devices.end(), message.name);
		if (tracked == tracked_devices.end()) {
			switch ( message.type ) {
		 		case typ::heartbeat: log(logging::warn, "Heartbeat recived from unknown device with name: '{}'", message.name);break;
		 		case typ::alarm: log(logging::critical, "Alarm message recived from unknown device with name: '{}'!", message.name);break;
		 		case typ::error:
		 		default: log(logging::warn, "unknown message/error [{}] recived from unknown device with name '{}'", message.type, message.name);break;
		 	}
		 	continue;
		}
		fmt::print("Tracked {} {}, message from {}\n", tracked->name, tracked->id, message.name);
		// tracked device found
		switch ( message.type ) {
		 case typ::heartbeat:{
			 /* Retrive the tracked device associated with the id in the message
			  * Update the tracked communication time with the current time.
			  */
			log(logging::info, "Recived Ok signal from '{}'", message.name);
			tracked->last_communication = now;
			tracked->first_detection = steady_clock::time_point::max();
			break;
		 }
		 case typ::alarm: {
			 /* Retrive the tracked device associated with the id in the message
			  * if it is not being tracked (time_point::max() is used as a sentinal if its not already tracked.)
			  * then set the first detection to now.
			  * if the current alarm has been going on longer than the timout time then send a alert.
			  */
		 	if ( tracked->first_detection == steady_clock::time_point::max() ) {
				// if first_detection is at sentinal value set it to now/ start counting down.
				log(logging::warn, "alarm started going off with name: '{}'", message.name);
				tracked->first_detection = now;
			}
			if ( now - tracked->first_detection > timeout_alarm_blaring ) {
				auto fire_alert = prepareAlert(tracked->id, message.type);
				this->sendAlert(fire_alert);
				// send repeated every timout
				// dev->first_detection = now;
				// send once unstil its been set to heartbeat and back.
				tracked->first_detection = steady_clock::time_point::max() - milliseconds{ 1 };
			}
			// make timout time equal to alarm time to make sure the ->alarm recived ->no communicaiton case happens much lower than the 3 minutes.
			tracked->last_communication = now - timeout_alarm_blaring + timeout_no_communication;
			break;
		 }
		 case typ::error: // all error types are sent with their error id...need to extend it for any error code added
		 default:{
			auto fire_alert = prepareAlert(tracked->id, message.type);
			this->sendAlert(fire_alert);
			break;
		 }
		}
	}
	// after all messages are processed remove them.
	messages.clear();
}

void Monitor::checkForTimeouts() {
	/* Itterate over all tracked devices and check to see if thire time_passed
	 * is greater than the defined timeout time in timeout_no_communication
	 * if it has timed out then prepare a alert message and send it to send_message
	 * if alarm was detected and timout time has been reached
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
		if (device.first_detection != max_time) {
			if ( (now - tracked->first_detection)*2 > timeout_alarm_blaring ) {
				auto fire_alert = prepareAlert(device.id, typ::alarm);
				this->sendAlert(fire_alert);
				// send repeated every timout
				// dev->first_detection = now;
				// send once unstil its ben set to heartbeat and back. (or timout)
				tracked->first_detection = steady_clock::time_point::max() - milliseconds{ 1 };
			}
		}
	}
}
