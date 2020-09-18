#include "logging.h"

namespace logging {
std::FILE* outFile; //global variable shared with openLogger() closeLogger() and log(...) functions
}
void openLogger() {
	using namespace std::chrono;
	auto now = system_clock::to_time_t(system_clock::now());
	// to remove "this function or variable may be unsafe" use buf and localtime_s instead
	tm buf;
	localtime_s(&buf, &now);
	char log_file_name[15];
	fmt::format_to_n(log_file_name, 14, "{:%Y_%m_%d}.log", buf);
	log_file_name[14] = '\0'; //format_to_n not leaving a \0 at the end of the string?

	fopen_s(&logging::outFile, log_file_name, "a");
	if (!logging::outFile ) {
		// the only time this should ever fail is if another program has a lock on the log file or if the program doesn't have correct permissions.
		fmt::print(fg(fmt::color::crimson), "ERROR Starting logger. Could not open file '{}'", log_file_name);
		fmt::print("\n\nsize of log_file_name {}", sizeof(log_file_name));
		std::exit(EXIT_FAILURE);
	} else {
		log(logging::info, "Loaded Logger");
	}
}
void closeLogger() {
	log(logging::info, "Closing Logger");
	std::fclose(logging::outFile);
}