#include "../headers/Communication.h"

// As gateway is situated on a rasberry pi the use case of messages is only set up for linux right now.
// In the future if the gateway is hosted on a different operating system then OS specific functions can easily be set up       
#ifdef __linux__
// mqtt code derived from mqtt example in linux folder.

void messageArrived(MQTT::MessageData& md) {
    MQTT::Message& message = md.message;

    log(logging::info, "Message arrived: qos {}, retained {}, dup {}, packetid {}\n",
            message.qos, message.retained, message.dup, message.id);
}

// IPStack and Countdown are found in ....../linux.cpp
void send_message(const char* msg, const char* hostname, const char* _clientID, const char* _username, const char* _password) {
    IPStack ipstack = IPStack();

    MQTT::Client<IPStack, Countdown> client = MQTT::Client<IPStack, Countdown>(ipstack);

    int port = 1883;
    int rc = ipstack.connect(hostname, port);
    if ( rc != 0 )
        log(logging::warn, "rc from TCP connect is {}\n", rc);

    log(logging::info, "MQTT connecting\n");
    MQTTPacket_connectData data = MQTTPacket_connectData_initializer;
    data.MQTTVersion = 3;
    data.clientID.cstring = (char*)_clientID;
    data.username.cstring = (char*)_username;
    data.password.cstring = (char*)_password;

    rc = client.connect(&data);
    if ( rc != 0 )
        log(logging::warn, "rc from MQTT connect is {}\n", rc);
    log(logging::info, "MQTT connected\n");

    rc = client.subscribe("+", MQTT::QOS2, messageArrived);
    if ( rc != 0 )
        log(logging::warn, "rc from MQTT subscribe is {}\n", rc);

    MQTT::Message message;

    // QoS 1 version
    message.qos = MQTT::QOS1;
    message.retained = false;
    message.dup = false;
    message.payload = (void*)msg;
    message.payloadlen = strlen(msg) + 1;
    
    rc = client.publish(topic, &message);
    while ( arrivedcount == 1 )
        client.yield(100);

    // QoS 2 version
    message.qos = MQTT::QOS2;
    rc = client.publish(topic, &message);
    while ( arrivedcount == 2 )
        client.yield(100);

    rc = client.unsubscribe(topic);
    if ( rc != 0 )
        log(logging::warn, "rc from unsubscribe was {}\n", rc);

    rc = client.disconnect();
    if ( rc != 0 )
        log(logging::warn, "rc from disconnect was {}\n", rc);

    ipstack.disconnect();
}

#else
void send_message(const char* msg, const char* hostname, const char* _clientID, const char* _username, const char* _password) {
	log(logging::warn, "Did not send message '{}' to '{}' due to platform not being supported. (debugging on windows or mac)\n", msg, hostname );
}
#endif // !__linux__
