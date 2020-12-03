#include "../headers/Communication.h"
#include "../headers/MonitorApp.h"

const int QOS = 1;
const int N_RETRY_ATTEMPTS = 5;

/////////////////////////////////////////////////////////////////////////////

// Callbacks for the success or failures of requested actions.
// This could be used to initiate further action, but here we just log the
// results to the console.

void action_listener::on_failure(const mqtt::token& tok) {
	log(logging::warn, "{} failure", name_);
	if (tok.get_message_id() != 0){
		log(logging::info, " ~>for token: [{}]", tok.get_message_id());
	}
}

void action_listener::on_success(const mqtt::token& tok) {
	log(logging::warn, "{} success", name_);
	if (tok.get_message_id() != 0){
		log(logging::info, " ~>for token: [{}]", tok.get_message_id());
	}
	auto top = tok.get_topics();
	if (top && !top->empty()){
		log(logging::info, " ~>token topic: '{}', ...", (*top)[0]);
	}
}

// This deomonstrates manually reconnecting to the broker by calling
// connect() again. This is a possibility for an application that keeps
// a copy of it's original connect_options, or if the app wants to
// reconnect with different options.
// Another way this can be done manually, if using the same options, is
// to just call the async_client::reconnect() method.
void ttn_connection_callback::reconnect() {
	std::this_thread::sleep_for(std::chrono::milliseconds(2500));
	try {
		cli_.connect(connOpts_, nullptr, *this);
	}
	catch (const mqtt::exception& exc) {
		log(logging::critical,"Error: {}\n", exc.what() );
	}
}

// Re-connection failure
void ttn_connection_callback::on_failure(const mqtt::token& tok) {
	if (++nretry_ > N_RETRY_ATTEMPTS){
		log(logging::critical, "Connection attempt failed.");
		return;
	}
	reconnect();
}

void ttn_connection_callback::connected(const std::string& cause) {
	log(logging::info, "Connection with ttn success- Subscribing to topic: {}\n", "fire/raw");

	cli_.subscribe("+/devices/+/up", QOS, nullptr, subListener_);
}

// Callback for when the connection is lost.
// This will initiate the attempt to manually reconnect.
void ttn_connection_callback::connection_lost(const std::string& cause) {
	log(logging::warn, "\nConnection to ttn lost");
	if (!cause.empty()){
		log(logging::info, "\tcause: {}", cause);
	}
	nretry_ = 0;
	reconnect();
}

// Callback for when a message arrives.
void ttn_connection_callback::message_arrived(mqtt::const_message_ptr msg) {
	log(logging::info, "\tpayload: '{}'", msg->to_string());
	auto lock = std::unique_lock<std::mutex>(m_ref);
	ref_recive_queue.emplace_back(typ::heartbeat, msg->to_string()); // add some massaging to make it fit the message type
	lock.unlock();
}


/////////////////////////////////////////////////////////////////////////////


// once I get Socket connection will have its own thread.
void Monitor::makeMessageThread() {
	using namespace std::chrono;
	auto connOpts = mqtt::connect_options_builder()
						.user_name(this->AppID)
						.password(this->AppKey)
						.automatic_reconnect(3s, 10s)
						.connect_timeout(60s)
						.finalize();

	ttn_sub = std::make_shared<mqtt::async_client>(this->ttn_host, this->ttnClientName);
	ttn_cbk = std::make_unique<ttn_connection_callback>(*ttn_sub, connOpts, this->recived, this->m);
	ttn_sub->set_callback(*ttn_cbk);

	// Start the connection.
	// When completed, the callback will subscribe to topic.
	
	try {
		// Note that we start the connection, but don't wait for completion.
		// We configured to allow publishing before a successful connection.
		log(logging::info, "Attempting to connect to ttn");
		ttn_sub->connect(connOpts, nullptr, *ttn_cbk);
	}catch (const mqtt::exception& exc) {
		log(logging::critical, "{}", exc.what());
	}

}

//Monitor::std::vector<Message> recived;

void Monitor::pollMessages() {
	// fake message sending for testing.
	
	auto lock = std::unique_lock<std::mutex>(m);
	if (!recived.empty()) {
		messages = std::move(recived);
	}
	lock.unlock();

}