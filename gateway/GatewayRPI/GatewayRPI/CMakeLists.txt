# First attempt at writing my own cmakelists. potentially written badly

cmake_minimum_required (VERSION 3.13)

if(UNIX AND NOT APPLE)
    set(LINUX TRUE)
endif()

message("Building project files")
# Add all source files to the GateLib library
add_library(GateLib STATIC GatewayRPI.h)

# Link libraries

add_subdirectory(headers)
add_subdirectory(src)


target_include_directories(GateLib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})


message("Building 3rd party")
cmake_policy(SET CMP0048 OLD) # does nothing :/ just ignore 0048 I don't want to edit the 3rd party cmake files.

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/lib/fmt/include)

# fmt header only library
# add_library(fmt INTERFACE)
# target_compile_definitions(fmt INTERFACE FMT_HEADER_ONLY=1)
# target_include_directories(fmt INTERFACE
#  ${CMAKE_CURRENT_SOURCE_DIR}/lib/fmt/include)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/lib/fmt)
target_link_libraries(GateLib fmt)

# same config as https://github.com/SRombauts/SQLiteCpp_Example/blob/master/CMakeLists.txt
set(SQLITECPP_RUN_CPPCHECK OFF CACHE BOOL "" FORCE)
set(SQLITECPP_RUN_CPPLINT OFF CACHE BOOL "" FORCE)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/lib/SQLite)

# Link gatelib with with SQLiteCpp and fmt
target_link_libraries(GateLib SQLiteCpp)


if(LINUX) # throws errors from all the linux specific code when I test in Visual studio...runs as normal on the pi
	# add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/lib/MQTT_C) # dependancy for the c++ mqtt
	# add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/lib/MQTT_CPP)

	include_directories(${CMAKE_CURRENT_SOURCE_DIR}/lib/RadioHead/)
endif()


# Please let me use c++11
target_compile_features(GateLib PUBLIC cxx_std_17)


# Add source to this project's executable.
add_executable (${PROJECT_NAME} main.cpp)

# add the gateway library to main
target_link_libraries(${PROJECT_NAME} PRIVATE GateLib)
