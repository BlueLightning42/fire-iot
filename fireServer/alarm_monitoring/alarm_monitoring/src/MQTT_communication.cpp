#include "../headers/Communication.h"
#include "../headers/MonitorApp.h"

// Example (heavily) edited from https://github.com/eclipse/paho.mqtt.cpp/blob/master/src/samples/async_publish_time.cpp
// and from https://github.com/eclipse/paho.mqtt.cpp/blob/master/src/samples/async_subscribe.cpp
// I'm sorry the code for the subscribing to ttn and the code for publishing to localhost are so different

namespace {
const int QOS_SEND = 1;
const int QOS_RECIVE = 1;
const int N_RETRY_ATTEMPTS = 5;
}

// structure setup like this/split between functions so its not baked into the Monitor and can be painlessly removed...
void inline Monitor::makeMessageSenderThread(){
	using namespace std::chrono;
	// How many messages to buffer while off-line
	const int MAX_BUFFERED_MESSAGES = 100;

	auto createOpts = mqtt::create_options_builder()
							.send_while_disconnected(true, true)
							.max_buffered_messages(MAX_BUFFERED_MESSAGES)
							.delete_oldest_messages()
							.finalize();

	cli = std::make_shared<mqtt::async_client>(this->host_name, this->client_name, createOpts);

	// Set callbacks for when connected and connection lost.
	cli->set_connected_handler([&](const std::string& s) {
		log(logging::info, "Connected to mqtt broker: {}", s);
		MQTT_connected = true;
	});
	cli->set_connection_lost_handler([&](const std::string& s) {
		log(logging::warn, "Lost connection to mqtt broker: {}", s);
		MQTT_connected = false;
	});
	cli->set_disconnected_handler([&](const mqtt::properties&, mqtt::ReasonCode code) {
		log(logging::info, "Disconnected from mqtt broker: code:{}", code);
		MQTT_connected = false;
	});

	auto last_will_text = fmt::format("Pi Monitor: '{}'' Disconnected", this->client_name);
	auto will_msg = mqtt::message(topic_name, last_will_text, 1, true);
	auto connOpts = mqtt::connect_options_builder()
						.will(will_msg)
						.user_name(this->username)
						.password(this->password)
						.automatic_reconnect(1s, 10s)
						.connect_timeout(5s)
						.finalize();

	try {
		// Note that we start the connection, but don't wait for completion.
		// We configured to allow publishing before a successful connection.
		log(logging::info, "Attempting to connect to mqtt broker @ {}", host_name);
		cli->connect(connOpts);
	}catch (const mqtt::exception& exc) {
		log(logging::critical, "{}", exc.what());
	}
}

void inline Monitor::makeMessageReciverThread() {
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

void Monitor::initMQTT(){
	makeMessageSenderThread();
	makeMessageReciverThread();
}

void Monitor::closeMQTT(){
	try{
		log(logging::info, "Attempting to disconnect from mqtt broker @ {}", host_name);
		cli->disconnect()->wait();
	}catch (const mqtt::exception& exc) {
		log(logging::critical, "{}", exc.what());
	}

	try {
		log(logging::info, "Attempting to disconnect from ttn");
		ttn_sub->disconnect()->wait();
	}catch (const mqtt::exception& exc) {
		log(logging::critical, "{}", exc.what());
	}
}


/* Callbacks for the success or failures of requested actions.
 * This could be used to initiate further action, but here we just log the
 * results to the console.
 */
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

/* This deomonstrates manually reconnecting to the broker by calling
 * connect() again. This is a possibility for an application that keeps
 * a copy of it's original connect_options, or if the app wants to
 * reconnect with different options.
 * Another way this can be done manually, if using the same options, is
 * to just call the async_client::reconnect() method.
 * for now leaving like this
 */
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

// Successfully connected
void ttn_connection_callback::connected(const std::string& cause) {
	log(logging::info, "Connection with ttn success- Subscribing to topic: {}", "fire/raw");

	cli_.subscribe("+/devices/+/up", QOS_RECIVE, nullptr, subListener_);
}

// Callback for when the connection is lost.
// This will initiate the attempt to manually reconnect.
void ttn_connection_callback::connection_lost(const std::string& cause) {
	log(logging::warn, "Connection to ttn lost");
	if (!cause.empty()){
		log(logging::info, "cause: {}", cause);
	}
	nretry_ = 0;
	reconnect();
}
