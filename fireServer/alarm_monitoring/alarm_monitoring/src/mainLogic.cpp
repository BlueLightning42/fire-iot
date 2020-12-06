#include "../headers/MonitorApp.h"

// main logic of the program

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
			checkDatabaseUpdate();
		 	tracked = std::find(tracked_devices.begin(), tracked_devices.end(), message.name);
			if (tracked == tracked_devices.end()) {
				switch ( message.type ) {
			 		case typ::heartbeat: log(logging::warn, "Heartbeat recived from unknown device with name: '{}'", message.name);break;
			 		case typ::alarm: log(logging::critical, "Alarm message recived from unknown device with name: '{}'!", message.name);break;
			 		case typ::error:
			 		default: log(logging::warn, "unknown message/error [{}] recived from unknown device with name '{}'", message.type, message.name);break;
			 	}
		 		continue;
		 	}
		}
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
			}else{
				log(logging::info, "recived another alarm message from '{}'", message.name);
			}
			if ( now - tracked->first_detection > timeout_alarm_blaring ) {
				auto fire_alert = prepareAlert(tracked->id, message.type);
				this->sendAlert(fire_alert);
				// send repeated every timout
				// dev->first_detection = now;
				// send once unstil its been set to heartbeat and back.
				tracked->first_detection = steady_clock::time_point::max() - 1ms ;
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
				fire_alert = prepareAlert(device.id, typ::no_communication_and_fire); // this logic could be changed to a seperate if statement if timout_no_communication is smaller with last message being an alarm
			}else {
				fire_alert = prepareAlert(device.id, typ::no_communication);
			}
			this->sendAlert(fire_alert);
			device.last_communication = max_time; // forces it to stop giving updates after one tick
			device.first_detection = max_time;    // turns off alarm as well
		}
		if (device.first_detection < max_time - 2ms ) {
			if ( (now - device.first_detection) > timeout_alarm_blaring*2 ) {
				auto fire_alert = prepareAlert(device.id, typ::alarm);
				this->sendAlert(fire_alert);
				// send repeated every timout
				// device.first_detection = now;
				// send once unstil its ben set to heartbeat and back. (or timout)
				device.first_detection = max_time - 1ms;
			}
		}
	}
}
