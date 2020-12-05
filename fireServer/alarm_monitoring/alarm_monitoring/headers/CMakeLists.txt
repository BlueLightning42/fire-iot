cmake_minimum_required (VERSION 3.13)

target_sources(MonitorLib
    PUBLIC
	    Communication.h
	    MonitorApp.h
	    Storage.h
		logging.h
)

target_include_directories(MonitorLib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
