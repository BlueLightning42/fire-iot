#include "../headers/MonitorApp.h"

// for the main logic of the program/mainloop visit "main_logic.cpp".

// the following section is just setting up Ctrl+C as a method of closing the program.
#ifdef __linux__
#include <signal.h>
#else // windows
	#ifndef NOMINMAX
	#define NOMINMAX //windows.h is a awful mess that defines gross coliding macros...that is all
	#endif //end NOMINMAX
#include <windows.h>
#endif //end __linux__

//Flag for Ctrl-C
namespace program{
#ifdef __linux__
	volatile sig_atomic_t running = true;
#else
	volatile bool running = true;
#endif
}
#ifdef __linux__
void sig_handler(int sig){
	log(logging::warn, "Break received, exiting!"); // probably not thread safe going remove in the future but for now leaving it.
  	program::running = false;
}
void setup_CTRLC() {
	signal(SIGINT, sig_handler); // Ctrl-C
}
#else // windows
BOOL WINAPI consoleHandler(DWORD signal) {
	if (signal == CTRL_C_EVENT) {
		log(logging::warn, "Break received, exiting!");
		program::running = false;
	}
	return TRUE;
}
void setup_CTRLC() {
	if (!SetConsoleCtrlHandler(consoleHandler, TRUE)){
		log(logging::critical, "Unable to setup Ctrl+C handler");
	}
}
#endif

// the follwing is simply setting up and cleaning up the program state
Monitor::Monitor(): MQTT_connected(false) {
	setup_CTRLC();
	openLogger(); // HAS to be opened before any logging can be done otherwise will crash on file write
	readConfig(); // Store some configurable values. (can be changed by users)
	loadDevices(); // gets a sorted vector of all the id's of stored devices.
	auto now = std::chrono::steady_clock::now();
	last_files_updated = now;
	last_logging_updated = now;
	initMQTT(); // inits both alert sender and TTN reciver.
	mainLoop();
}
Monitor::~Monitor() {
	// storeDevices(tracked_devices); // store last state of devices.
	closeMQTT();
	closeLogger(); // Good manners to close logging resources...the operating system probably can if its shutoff but its good manners all the same.
}

void Monitor::sendAlert(const std::string& msg) {
	// makes it easier to replace MQTT by swapping out this part...I like how its setup with MQTT but if someone wants to swap it for some other technology its here.
	send_MQTT_message(msg);
}

void Monitor::mainLoop() {
	/* Endless loop for continous program.
	 * check for new messages.
	 * if messages recived update all the devices.
	 * then check for any timeouts
	 * check if the config file or the database file have changed every 2min
	 * repeat.
	 */
	while ( program::running ) {
		pollMessages(); // in Packet_communication.cpp
		if ( !messages.empty() ) {
			updateTrackedDevices(); // in mainLogic.cpp
		}
		checkForTimeouts(); // in mainLogic.cpp
		periodicReset(); // in Storage.cpp
	}
}
