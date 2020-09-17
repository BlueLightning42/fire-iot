#pragma once

#pragma warning( disable : 26812 26451 26444 6387 26498 26495) 

// standard library headers
#include <stdint.h> //uint32_t etc 
#include <vector>
#include <utility> // pair
#include <chrono> // system_clock steady_clock
#include <ctime>   // localtime_s

// external library headers

//prefered console library. 
#include <fmt/format.h>
#include <fmt/core.h>
#include <fmt/color.h>
#include <fmt/chrono.h>

//storage of user info from signup page.
#include <sqlite3.h>

// utility logging function

namespace logging {
enum error {
	info,
	warn,
	critical // potential cause of a crash
};
}
template<typename Str, typename ...Args>
void log(logging::error err, Str format, const Args& ...args) {
	using namespace std::chrono;
	auto now = system_clock::to_time_t(system_clock::now());

	tm buf; // to remove "this function or variable may be unsafe" use localtime_s instead
	localtime_s(&buf, &now);
	fmt::print(fg(fmt::color::dim_gray), "[{:%Y-%m-%d %H:%M:%S}]> ", buf); 

	if ( err == logging::info ) { //only for console?
		fmt::print( fg(fmt::color::dim_gray), format, args...);
	} else if ( err == logging::warn ) {
		fmt::print(fg(fmt::color::yellow), format, args...);
	}else if ( err == logging::critical ) { //an error that could crash the program.
		fmt::print(fg(fmt::color::crimson) | fmt::emphasis::bold, format, args...);
	}
}

