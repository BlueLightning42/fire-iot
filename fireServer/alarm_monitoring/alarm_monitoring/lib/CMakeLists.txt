# The absolute worst cmake file in this project...everything has been changed 100x before it worked and they all use different "hacks" to run

cmake_minimum_required (VERSION 3.13)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/fmt/include)
target_include_directories(MonitorLib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/rapidjson/include)

# fmt header only library
# add_library(fmt INTERFACE)
# target_compile_definitions(fmt INTERFACE FMT_HEADER_ONLY=1)
# target_include_directories(fmt INTERFACE
#  ${CMAKE_CURRENT_SOURCE_DIR}/fmt/include)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/fmt)
target_link_libraries(MonitorLib fmt)

# same config as https://github.com/SRombauts/SQLiteCpp_Example/blob/master/CMakeLists.txt
set(SQLITECPP_RUN_CPPCHECK OFF CACHE BOOL "" FORCE)
set(SQLITECPP_RUN_CPPLINT OFF CACHE BOOL "" FORCE)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/SQLite)

# Link gatelib with with SQLiteCpp
target_link_libraries(MonitorLib SQLiteCpp)

if(WIN32)
	# absolute paths are garbage but windows is garbage as well so w/e this library is annoying to build
	set(CMAKE_PREFIX_PATH "C:/mqtt/paho-c")
endif()
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/MQTT_CPP)
include_directories(${RadioHeadDir}/MQTT_CPP/src)
if(LINUX)
	target_link_libraries(MonitorLib paho-mqttpp3)
else()
	target_link_libraries(MonitorLib paho-mqttpp3-static)
endif()
