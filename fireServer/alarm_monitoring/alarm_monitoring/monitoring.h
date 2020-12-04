#pragma once

#pragma warning( disable : 26812 26451 26444 6387 26498 26495) 

// standard library headers
#include <stdint.h> //uint32_t etc 
#include <vector>
#include <chrono>	// system_clock steady_clock

#include <cstdio>
#include <string>
#include <filesystem>
#include <cstring>

#include <fstream>

#include <thread>
#include <mutex>
// external library headers

//prefered console library. Used to speed up dev of logging/debugging...
#include <fmt/core.h>
#include <fmt/format.h>
#include <fmt/os.h>
#include <fmt/color.h>
#include <fmt/chrono.h>

//storage of user info from signup page. Using a wrapper for sqlite3
#include <SQLiteCpp/SQLiteCpp.h>

// utility logging function
#include "headers/logging.h"


// communication stuff...probably some of the more complicated parts of this

// TODO: decide if its simpler to use QOS1 and figure it out on the fire department end.
// Reminder
// QOS0 = at most once.
// QOS1 = at least once.
// QOS2 = exactly once.


#include <mqtt/async_client.h>
#define RAPIDJSON_HAS_STDSTRING 1
#include <rapidjson/document.h>