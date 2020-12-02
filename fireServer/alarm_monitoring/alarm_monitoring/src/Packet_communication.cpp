#include "../headers/Communication.h"
#include "../headers/MonitorApp.h"

const std::string SERVER_ADDRESS("tcp://localhost:1883");
const std::string CLIENT_ID("async_subcribe_cpp");
const std::string TOPIC("hello");

const int	QOS = 1;
const int	N_RETRY_ATTEMPTS = 5;

/////////////////////////////////////////////////////////////////////////////

// Callbacks for the success or failures of requested actions.
// This could be used to initiate further action, but here we just log the
// results to the console.

class action_listener : public virtual mqtt::iaction_listener{
	std::string name_;

	void on_failure(const mqtt::token& tok) override {
		log(logging::warn, "{} failure", _name);
		if (tok.get_message_id() != 0){
			log(logging::info, " for token [{}]", tok.get_message_id());
		}
	}

	void on_success(const mqtt::token& tok) override {
		log(logging::info, "{} success", _name);
		if (tok.get_message_id() != 0){
			log(logging::info, " for token [{}]", tok.get_message_id());
		}
		auto top = tok.get_topics();
		if (top && !top->empty()){
			log(logging::info, "token topic: {}, ...\n", (*top)[0]);
		}
	}

public:
	action_listener(const std::string& name) : name_(name) {}
};

/////////////////////////////////////////////////////////////////////////////

/**
 * Local callback & listener class for use with the client connection.
 * This is primarily intended to receive messages, but it will also monitor
 * the connection to the broker. If the connection is lost, it will attempt
 * to restore the connection and re-subscribe to the topic.
 */
class callback : public virtual mqtt::callback,
					public virtual mqtt::iaction_listener{
	// Counter for the number of connection retries
	int nretry_;
	// The MQTT client
	mqtt::async_client& cli_;
	// Options to use if we need to reconnect
	mqtt::connect_options& connOpts_;
	// An action listener to display the result of actions.
	action_listener subListener_;

	// This deomonstrates manually reconnecting to the broker by calling
	// connect() again. This is a possibility for an application that keeps
	// a copy of it's original connect_options, or if the app wants to
	// reconnect with different options.
	// Another way this can be done manually, if using the same options, is
	// to just call the async_client::reconnect() method.
	void reconnect() {
		std::this_thread::sleep_for(std::chrono::milliseconds(2500));
		try {
			cli_.connect(connOpts_, nullptr, *this);
		}
		catch (const mqtt::exception& exc) {
			log(logging::critical,"Error: {}\n", exc.what() );
		}
	}

	// Re-connection failure
	void on_failure(const mqtt::token& tok) override {
		log(logging::warn, "Connection attempt failed.");
		if (++nretry_ > N_RETRY_ATTEMPTS)
			exit(1);
		reconnect();
	}

	// (Re)connection success
	// Either this or connected() can be used for callbacks.
	void on_success(const mqtt::token& tok) override {}

	// (Re)connection success
	void connected(const std::string& cause) override {
		log(logging::info, "Connection success\nSubscribing to topic: {}\n", TOPIC);
		log(logging::info, "\tfor client: {} using QoS {}\n",CLIENT_ID, QOS)

		cli_.subscribe(TOPIC, QOS, nullptr, subListener_);
	}

	// Callback for when the connection is lost.
	// This will initiate the attempt to manually reconnect.
	void connection_lost(const std::string& cause) override {
		log(logging::info, "\nConnection lost");
		if (!cause.empty()){
			log(logging::info, "\tcause: {}", cause)
		}
		nretry_ = 0;
		reconnect();
	}

	// Callback for when a message arrives.
	void message_arrived(mqtt::const_message_ptr msg) override {
		log(logging::info, "\tpayload: '{}'", msg->to_string());
	}

	void delivery_complete(mqtt::delivery_token_ptr token) override {}

public:
	callback(mqtt::async_client& cli, mqtt::connect_options& connOpts)
				: nretry_(0), cli_(cli), connOpts_(connOpts), subListener_("Subscription") {}
};

/////////////////////////////////////////////////////////////////////////////




#ifdef __linux__


void initSockets(){
	
}

void Monitor::pollMessages() {

}
// once I get Socket connection will have its own thread.
void Monitor::makeMessageThread() {
	message_thread = std::thread([&] {
		const char* ttn = "us-west.thethings.network:1883";
		const char* AppID = "fire-detection-iot";
		const char* AppKey = "ttn-account-v2.sXLemq1_Q99MjfKoYSc2BpAUTMDuVdgnRYcvRk8PFAs";

		using namespace std::chrono;

		// How many messages to buffer while off-line
		
		



	});
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