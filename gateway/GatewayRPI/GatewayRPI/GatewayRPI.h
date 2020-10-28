#pragma once

#pragma warning( disable : 26812 26451 26444 6387 26498 26495) 

// standard library headers
#include <stdint.h> //uint32_t etc 
#include <vector>
#include <chrono>	// system_clock steady_clock
#include <ctime>	// localtime_s (just used for logging...and theres a chance I can remove it with fmt chrono
// #include <fstream>	// ofstream was annoying me with not being able to implement a global/static object
#include <cstdio>
#include <string>
#include <fstream>

// external library headers

//prefered console library. Used to speed up dev of logging/debugging...
#define FMT_HEADER_ONLY 1
#include "fmt/format.h"
#include "fmt/color.h"
#include "fmt/chrono.h"

//storage of user info from signup page. Using a wrapper for sqlite3
#include <SQLiteCpp/SQLiteCpp.h>

// utility logging function
#include "headers/logging.h"



// communication stuff...probably some of the more complicated parts of this

#ifdef __linux__
// TODO: decide if its simpler to use QOS1 and figure it out on the fire department end.
// Reminder
// QOS0 = at most once.
// QOS1 = at least once.
// QOS2 = exactly once.

#define MQTTCLIENT_QOS2 1
//communication with fire department over mqtt
#include "./lib/mqtt/MQTTClient/src/MQTTClient.h"

#include "./lib/mqtt/MQTTClient/src/linux/linux.cpp"

// initial attempt https://www.airspayce.com/mikem/arduino/RadioHead/ if this doesn't run on the pi going use this fork https://github.com/hallard/RadioHead (much older commit so trying new official first)
// I need a driver and a manager
// RH_RF95 Works with Semtech SX1276/77/78/79, Modtronix inAir4 and inAir9, and HopeRF RFM95/96/97/98 and other similar LoRa capable radios. Supports Long Range (LoRa) with spread spectrum frequency hopping, large payloads etc. FSK/GFSK/OOK modes are not (yet) supported.
// https://www.airspayce.com/mikem/bcm2835/ needs to be downloaded and init...cant figure out if I can make a submodule yet
#include "./lib/RadioHead/RH_RF95.h" // I think I'm going have to use either this without the manager or
#include "./lib/RadioHead/RHDatagram.h"

#endif

