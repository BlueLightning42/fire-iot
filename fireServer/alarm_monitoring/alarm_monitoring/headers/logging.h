#pragma once

#include "../monitoring.h"

// This file is just for pretty printing logs to standard output and a file...probably full of horrible hacks its just to make debugging easier for me.
// should probably be wrapped in a class but I don't want to pass a logging object around to every single function that might print something
// and having a global object that still has to have a templated function/now its a templated method

namespace logging {
enum error {
	info,
	warn,
	critical // potential cause of a crash
};
extern std::FILE* outFile; //external linkage cause of the stupid LNK4006
static std::mutex log_mut;
}


// left in header instead of .cpp because I dont remember how to fix template garbage outside of a header

template<typename Str, typename ...Args>
void log(logging::error err, Str format, const Args& ...args) {
#ifdef NO_LOG_INFO
	if(err == error::info) return;
#endif
	using namespace std::chrono;

	// I get the multi threading "just throw a lock on it" concept isn't the best for performance
	// but logs are infrequent and I don't really want to focus on this when I need to fix the actual application.
	auto lock = std::unique_lock<std::mutex>(logging::log_mut);
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

	lock.unlock();
}
// similar function when theres no arguments to format
template<typename Str>
void log(logging::error err, Str statement) {
#ifdef NO_LOG_INFO
	if(err == error::info) return;
#endif
	using namespace std::chrono;

	auto lock = std::unique_lock<std::mutex>(logging::log_mut);
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

	lock.unlock();
}
void openLogger();
void closeLogger();
