#include "../headers/Communication.h"
#include "../headers/MonitorApp.h"
#include "../headers/base64decode.h"

const int QOS = 1;
const int N_RETRY_ATTEMPTS = 5;

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
	using namespace rapidjson;
	Document doc;
	const std::string& json = msg->to_string();
	if (doc.Parse(json).HasParseError()){
		log(logging::critical, "Parse error while parsing ttn payload.");
		log(logging::info, "\tpayload: '{}'", msg->to_string());
        return;
	}
	Value::MemberIterator nameit = doc.FindMember("dev_id");
	if(nameit == doc.MemberEnd()){
		log(logging::critical, "ttn payload doesn't contain name"); return;
	}else if(!nameit->value.IsString()){
		log(logging::critical, "ttn dev_id not a string."); return;
	}
	std::string name = nameit->value.GetString();

	nameit = doc.FindMember("payload_raw");
	if(nameit == doc.MemberEnd()){
		log(logging::critical, "ttn payload_raw doesn't exist"); return;
	}else if(!nameit->value.IsString()){
		log(logging::critical, "ttn payload_raw not a string."); return;
	}
	typ::Type payload;
	/* TTN sends payload packets that are base 64 encoded and the bytes are hexadeciaml.
	 * - 00 sent as AA== and 01 sent as AQ==
	 * Please do not think this is how base64 strings should normally be decoded
	 * its just something I wrote quickly to get the data...considering the top two versions are more likly to happen
	 * and considering they should happen quite a few times its not too much of a premature optimization.
	 * If we figure out the end point and send packets using a value like "code: 1" then I can find it in the json payload and don't have to decode it from the payload_raw
	 */
	if(strcmp("AA==", nameit->value.GetString()) == 0){
		payload = typ::heartbeat;
	}else if(strcmp("AQ==", nameit->value.GetString()) == 0){
		payload = typ::alarm;
	}else{
		std::string payload_raw = b64decode(nameit->value.GetString());
		log(logging::info, "non heartbeat/alarm code recived with: {}", (int)payload_raw[0]);
		if(payload_raw.size() != 1){
			log(logging::warn, "payload_raw is larger than 1 byte");
			payload = typ::error;
		}else{
			payload = (typ::Type)payload_raw[0];
		}
	}

	log(logging::info, "name is {}", name);
	auto lock = std::unique_lock<std::mutex>(m_ref);
	ref_recive_queue.emplace_back(payload, name); // add some massaging to make it fit the message type
	lock.unlock();
}


/////////////////////////////////////////////////////////////////////////////


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

//Monitor::std::vector<Message> recived; (is the format of the recived vector)

void Monitor::pollMessages() {
	// fake message sending for testing.

	auto lock = std::unique_lock<std::mutex>(m);
	if (!recived.empty()) {
		messages = std::move(recived);
	}
	lock.unlock();

}
