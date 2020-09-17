cmake_minimum_required (VERSION 3.13)

target_sources(GateLib
    PUBLIC
	    Communication.h
	    Gateway.h
	    Storage.h
)

target_include_directories(GateLib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})