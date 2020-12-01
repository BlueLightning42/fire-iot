#pragma once

#include "../monitoring.h"

// this file is just for pretty printing logs. 

namespace logging {
enum error {
	info,
	warn,
	critical // potential cause of a crash
};
extern std::FILE* outFile; //external linkage cause of the stupid LNK4006
}


// left in header instead of .cpp because I dont remember how to fix template garbage outside of a header

template<typename Str, typename ...Args>
void log(logging::error err, Str format, const Args& ...args) {
	using namespace std::chrono;
	auto now = system_clock::to_time_t(system_clock::now());
	// Adding a timestamp to the start of a log
	fmt::print(fg(fmt::color::dim_gray), "[{:%Y-%m-%d %H:%M:%S}]> ", fmt::localtime(now));

	if ( err == logging::info ) { //only for console?
		fmt::print(fg(fmt::color::dim_gray), format, args...);
	} else if ( err == logging::warn ) {
		fmt::print(fg(fmt::color::yellow), format, args...);
	} else if ( err == logging::critical ) { //an error that could crash the program.
		fmt::print(fg(fmt::color::crimson) | fmt::emphasis::bold, format, args...);
	}
	fmt::print("\n");

	std::string temp = fmt::format("[{:%Y-%m-%d %H:%M:%S}]> {}\n", fmt::localtime(now), format);
	fmt::print(logging::outFile, temp.c_str(), args...);
}
// similar function when theres no arguments to format
template<typename Str>
void log(logging::error err, Str statement) {
	using namespace std::chrono;
	auto now = system_clock::to_time_t(system_clock::now());
	fmt::print(fg(fmt::color::dim_gray), "[{:%Y-%m-%d %H:%M:%S}]> ", fmt::localtime(now));

	if ( err == logging::info ) { //only for console? or both it and file? 
		fmt::print(fg(fmt::color::dim_gray), statement);
	} else if ( err == logging::warn ) { // Something that the user should be aware about
		fmt::print(fg(fmt::color::yellow), statement);
	} else if ( err == logging::critical ) { //an error that could crash the program.
		fmt::print(fg(fmt::color::crimson) | fmt::emphasis::bold, statement);
	}
	fmt::print("\n");

	fmt::print(logging::outFile, "[{:%Y-%m-%d %H:%M:%S}]> {}\n", fmt::localtime(now), statement);
}
void openLogger();
void closeLogger();
