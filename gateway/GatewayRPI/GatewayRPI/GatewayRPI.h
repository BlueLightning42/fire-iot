﻿#pragma once

#pragma warning( disable : 26812 26451 26444 6387 26498 26495) 

// standard library headers
#include <stdint.h> //uint32_t etc 
#include <vector>
#include <chrono>	// system_clock steady_clock
#include <ctime>	// localtime_s
// #include <fstream>	// ofstream was annoying me with not being able to implement a global/static object
#include <cstdio>
#include <string>

// external library headers

//prefered console library. Used to speed up dev of logging/debugging...
#include <fmt/format.h>
#include <fmt/color.h>
#include <fmt/chrono.h>

// utility logging function 
#include "headers/logging.h"

//storage of user info from signup page.
#include <sqlite3.h>

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
#endif

