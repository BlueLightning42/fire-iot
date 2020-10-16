#include "../headers/Gateway.h"


Gateway::Gateway(): running(true), message_recived(false) {
	openLogger();
	tracked_devices = loadDevices();
}
Gateway::~Gateway() {
	storeDevices(tracked_devices);
	closeLogger();
}
void Gateway::mainLoop() {
	while ( running ) {
		pollMessages();
		if ( message_recived ) {
			updateTrackedDevices();
		}
		checkForTimeouts();
	}
}

inline void Gateway::pollMessages() {

}


void Gateway::updateTrackedDevices() {
	for ( const auto& message : messages ) {
		switch ( message.type ) {
		 case typ::heartbeat:{
			auto to_update = std::lower_bound(tracked_devices.begin(), tracked_devices.end(), message.id);
			if ( to_update->id != message.id ) {
				log(logging::info, "Recived Ok signal from {}", message.id);
				to_update->last_communication = std::chrono::steady_clock::now();
			}
			break;
		 }
		 case typ::alarm:{
			 auto fire_alert = prepareAlert(message.id, message.type);
			 send_message(fire_alert.c_str(), host_name.c_str());
			 break;
		 }
		 case typ::error:{
			auto fire_alert = prepareAlert(message.id, message.type);
			send_message(fire_alert.c_str(), host_name.c_str());
			break;
		 }
		 default:{
			 log(logging::critical, "unknown message type for message id:{}", message.id);
		 }
		}
	}
}

void Gateway::checkForTimeouts() {
	for ( const auto device : tracked_devices ) {
		auto time_passed = std::chrono::steady_clock::now() - device.last_communication;
		if ( time_passed > this->timeout_no_communication ) {
			auto fire_alert = prepareAlert(device.id, typ::no_communication);
			send_message(fire_alert.c_str(), host_name.c_str());
		}
	}
}