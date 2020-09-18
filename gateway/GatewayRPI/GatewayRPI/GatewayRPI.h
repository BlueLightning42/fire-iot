#pragma once

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

//prefered console library. 
#include <fmt/format.h>
#include <fmt/color.h>
#include <fmt/chrono.h>

//storage of user info from signup page.
#include <sqlite3.h>

// utility logging function
#include "headers/logging.h"
