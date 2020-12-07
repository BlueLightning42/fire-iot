#pragma once

#include "../monitoring.h"

namespace typ{
enum Type: unsigned char {
	heartbeat,	// 00
	alarm,		// 01
	error,		// 10 ? (see if more error types are needed)
	no_communication, // gateway error...should not be sent from edge device idk
	no_communication_and_fire
};
}
/*
format of the full packet recived through Who knows at this point.
*/
struct Message {
	typ::Type type;// : 4; //bits indicating the state of the communicating alarm.
	std::string name;
	std::string extra;
	Message(typ::Type t, std::string _name, std::string _extra): type(t), name(_name), extra(_extra) {}
	Message(): type(typ::error), name("_"), extra("") {}
};

class action_listener : public virtual mqtt::iaction_listener{
 private:
	std::string name_;
	void on_failure(const mqtt::token& tok) override;
	void on_success(const mqtt::token& tok) override;
 public:
	action_listener(const std::string& name) : name_(name) {}
	~action_listener() override {}
};


/**
 * Local callback & listener class for use with the client connection.
 * This is primarily intended to receive messages, but it will also monitor
 * the connection to the broker. If the connection is lost, it will attempt
 * to restore the connection and re-subscribe to the topic.
 */
class ttn_connection_callback : public virtual mqtt::callback,
								public virtual mqtt::iaction_listener{
 private:
	int nretry_; // Counter for the number of connection retries
	mqtt::async_client& cli_; //mqtt connection
	mqtt::connect_options connOpts_;// Options to use if we need to reconnect
	action_listener subListener_;
	std::vector<Message> &ref_recive_queue;
	std::mutex &m_ref;

	void reconnect();
	// Re-connection failure
	void on_failure(const mqtt::token& tok) override;
	// (Re)connection success
	// Either this or connected() can be used for callbacks.
	void on_success(const mqtt::token& tok) override {}
	// (Re)connection success
	void connected(const std::string& cause) override;
	// Callback for when the connection is lost.
	// This will initiate the attempt to manually reconnect.
	void connection_lost(const std::string& cause) override;
	// Callback for when a message arrives.
	void message_arrived(mqtt::const_message_ptr msg) override;
	void delivery_complete(mqtt::delivery_token_ptr token) override {}
 public:
	ttn_connection_callback(mqtt::async_client& cli, mqtt::connect_options& co, std::vector<Message> &rrq, std::mutex &mr)
				: nretry_(0), cli_(cli), connOpts_(co), ref_recive_queue(rrq), m_ref(mr), subListener_("TTN Subscription") {}
	~ttn_connection_callback() {}
};
