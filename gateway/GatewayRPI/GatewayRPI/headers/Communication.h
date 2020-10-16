#pragma once

#include "../GatewayRPI.h"

namespace typ{
enum Type: unsigned {
	heartbeat,	// 00
	alarm,		// 01
	error,		// 10 ? (see if more error types are needed)
	no_communication // gateway error...should not be sent from edge device idk
};
}
/*
format of the full packet recived through LoRa
*/
struct Message {
	uint16_t dest; // id of the desired gateway?
	uint16_t id; // id of the incomming alarm
	typ::Type type : 2; //bits indicating the state of the communicating alarm.
	unsigned padding : 6; // TODO pad the rest of the byte with info?
};

void send_message(const char* msg, const char* hostname, const char* _clientID, const char* _username, const char* _password);