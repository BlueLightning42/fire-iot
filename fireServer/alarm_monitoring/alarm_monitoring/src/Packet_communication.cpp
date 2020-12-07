#include "../headers/Communication.h"
#include "../headers/MonitorApp.h"
#include "../headers/base64decode.h"

namespace {
const int QOS_SEND = 1;
}

// Callback for when a message arrives.
void ttn_connection_callback::message_arrived(mqtt::const_message_ptr msg) {
	using namespace rapidjson;
	Document doc;
	const std::string& json = msg->to_string();
	if (doc.Parse(json).HasParseError()){
		log(logging::critical, "Parse error while parsing ttn payload.");
		log(logging::info, "->\tpayload: '{}'", msg->to_string());
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
	std::string extra = "";
	nameit = doc.FindMember("metadata");
	if(nameit != doc.MemberEnd()){
		// I dont think most devices are sending the information so 99% of the time its empty
		Value::MemberIterator latlong = nameit->value.FindMember("latitude");
		if(latlong != nameit->value.MemberEnd()){
			extra = fmt::format("latitude: {},", latlong->value.GetDouble());
		}
		latlong = nameit->value.FindMember("longitude");
		if(latlong != nameit->value.MemberEnd()){
			auto tmp = fmt::format("\n[{} | longitude: {}]",extra, latlong->value.GetDouble());
			extra = tmp;
			log(logging::info, "received coords {}", extra);
		}
	}

	auto lock = std::unique_lock<std::mutex>(m_ref);
	ref_recive_queue.emplace_back(payload, name, extra);
	lock.unlock();
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

/////////////////////////////////////////////////////////////////////////////

// send messages

void Monitor::send_MQTT_message(const std::string& msg) {
	if( MQTT_connected == false ){
		// if not connected already try to connect again.
		initMQTT();
	}
	try {
		auto top = mqtt::topic(*cli, topic_name, QOS_SEND);
		top.publish(msg);
		log(logging::info, "message `{}`, send to topic {}", msg, topic_name);
	}
	catch (const mqtt::exception& exc) {
		log(logging::critical, "{}", exc.what());
	}
}
