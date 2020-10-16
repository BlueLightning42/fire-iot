cmake_minimum_required (VERSION 3.13)

target_sources(GateLib
    PUBLIC
	    MQTT_communication.cpp
	    LoRa_communication.cpp
	    Gateway.cpp
	    Storage.cpp
		logging.cpp
)