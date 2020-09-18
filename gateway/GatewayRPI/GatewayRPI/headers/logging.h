#pragma once

#include "../GatewayRPI.h"

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

	// to remove "this function or variable may be unsafe" using localtime_s instead
	tm buf;
	localtime_s(&buf, &now);
	// Adding a timestamp to the start of a log
	fmt::print(fg(fmt::color::dim_gray), "[{:%Y-%m-%d %H:%M:%S}]> ", buf);

	if ( err == logging::info ) { //only for console?
		fmt::print(fg(fmt::color::dim_gray), format, args...);
	} else if ( err == logging::warn ) {
		fmt::print(fg(fmt::color::yellow), format, args...);
	} else if ( err == logging::critical ) { //an error that could crash the program.
		fmt::print(fg(fmt::color::crimson) | fmt::emphasis::bold, format, args...);
	}
	fmt::print("\n");

	std::string temp = fmt::format("[{:%Y-%m-%d %H:%M:%S}]> {}\n", buf, format);
	fmt::print(logging::outFile, temp.c_str(), args...);
}
// similar function when theres no arguments to format
template<typename Str>
void log(logging::error err, Str statement) {
	using namespace std::chrono;
	auto now = system_clock::to_time_t(system_clock::now());

	tm buf; // to remove "this function or variable may be unsafe" use localtime_s instead
	localtime_s(&buf, &now);
	fmt::print(fg(fmt::color::dim_gray), "[{:%Y-%m-%d %H:%M:%S}]> ", buf);

	if ( err == logging::info ) { //only for console?
		fmt::print(fg(fmt::color::dim_gray), statement);
	} else if ( err == logging::warn ) {
		fmt::print(fg(fmt::color::yellow), statement);
	} else if ( err == logging::critical ) { //an error that could crash the program.
		fmt::print(fg(fmt::color::crimson) | fmt::emphasis::bold, statement);
	}
	fmt::print("\n");

	fmt::print(logging::outFile, "[{:%Y-%m-%d %H:%M:%S}]> {}\n", buf, statement);
}
void openLogger();
void closeLogger();
