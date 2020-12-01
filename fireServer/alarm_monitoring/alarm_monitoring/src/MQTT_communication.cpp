#include "../headers/Communication.h"
#include "../headers/MonitorApp.h"

// Example (heavily) edited from https://github.com/eclipse/paho.mqtt.cpp/blob/master/src/samples/async_publish_time.cpp

// structure setup like this/split between functions so its not baked into the Monitor and can be painlessly removed...
void Monitor::initMQTT(){
	using namespace std::chrono;

	// How many messages to buffer while off-line
	const int MAX_BUFFERED_MESSAGES = 100;

	// host_name = "broker.hivemq.com"; // testing server.

	auto createOpts = mqtt::create_options_builder()
						  .send_while_disconnected(true, true)
					      .max_buffered_messages(MAX_BUFFERED_MESSAGES)
						  .delete_oldest_messages()
						  .finalize();

	// switch to mqtt::async_client_ptr if this doesn't work
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

void Monitor::closeMQTT(){
	try{
		log(logging::info, "Attempting to disconnect from mqtt broker @ {}", host_name);
		cli->disconnect()->wait();
	}catch (const mqtt::exception& exc) {
		log(logging::critical, "{}", exc.what());
	}
}

void Monitor::send_MQTT_message(const std::string& msg) {
	if( MQTT_connected == false ){
		// if not connected already try to connect again.
		initMQTT();
	}
	// The QoS for sending data
	const int QOS = 1;

	try {
		auto top = mqtt::topic(*cli, topic_name, QOS);
		
		top.publish(msg);
	}
	catch (const mqtt::exception& exc) {
		log(logging::critical, "{}", exc.what());
	}
}
