cmake_minimum_required (VERSION 3.13)

target_sources(MonitorLib
    PUBLIC
	    MQTT_communication.cpp
	    Packet_communication.cpp
	    MonitorApp.cpp
	    Storage.cpp
		logging.cpp
)
