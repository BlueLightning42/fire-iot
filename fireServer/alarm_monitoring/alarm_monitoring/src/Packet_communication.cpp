#include "../headers/Communication.h"
#include "../headers/MonitorApp.h"

#ifdef __linux__


void initSockets(){
	
}

void Monitor::pollMessages() {

}
// once I get Socket connection will have its own thread.
void Monitor::makeMessageThread() {

}

#else
void initSockets() {
	log(logging::warn, "Running on windows- Socket functions are not setup (yet?)");
}
#include <iostream>

//Monitor::std::vector<Message> recived;

void Monitor::makeMessageThread() {
	message_thread = std::thread([&] {
		std::string s;
		bool error = false;
		while (!error && std::getline(std::cin, s, '\n')) {
			auto lock = std::unique_lock<std::mutex>(m);
			message_recived = true;
			if (s == "quit") {
				error = true;
			}
			auto type = typ::heartbeat;
			if (s.rfind("a", 0) == 0) {
				type = typ::alarm;
				s = s.substr(1);
			}
			try {
				uint16_t num = std::stoi(s);
				Message fake{0, num, type, 0 };
				recived.emplace_back(fake);
			}
			catch (...) {}
			lock.unlock();
		}
	});
}
void Monitor::pollMessages() {
	// fake message sending for testing.
	
	auto lock = std::unique_lock<std::mutex>(m);
	if (message_recived) {
		messages = std::move(recived);
		message_recived = false;
	}
	lock.unlock();

}
#endif