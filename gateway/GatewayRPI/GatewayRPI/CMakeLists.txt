# First attempt at writing my own cmakelists. potentially written badly

cmake_minimum_required (VERSION 3.13)

if(UNIX AND NOT APPLE)
    set(LINUX TRUE)
endif()

# Add all source files to the GateLib library
add_library(GateLib STATIC GatewayRPI.h)

# Link libraries

add_subdirectory(headers)
add_subdirectory(src)


target_include_directories(GateLib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/lib/fmt/include)


cmake_policy(SET CMP0048 OLD)
# same config as https://github.com/SRombauts/SQLiteCpp_Example/blob/master/CMakeLists.txt
set(SQLITECPP_RUN_CPPCHECK OFF CACHE BOOL "" FORCE)
set(SQLITECPP_RUN_CPPLINT OFF CACHE BOOL "" FORCE)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/lib/SQLite)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/lib/mqtt/)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/lib/fmt)

# Link gatelib with with SQLiteCpp
target_link_libraries(GateLib SQLiteCpp)
target_link_libraries(GateLib SQLiteCpp)
target_link_libraries(GateLib SQLiteCpp)


if(LINUX)
	target_include_directories(GateLib PRIVATE 
		${CMAKE_CURRENT_SOURCE_DIR}/lib/mqtt/MQTTClient/src 
		${CMAKE_CURRENT_SOURCE_DIR}/lib/mqtt/MQTTClient/src/linux
	)
endif()


# Please let me use c++11
target_compile_features(GateLib PUBLIC cxx_std_11)


# Add source to this project's executable.
add_executable (${PROJECT_NAME} main.cpp)

# add the gateway library to main
target_link_libraries(${PROJECT_NAME} PRIVATE GateLib)
