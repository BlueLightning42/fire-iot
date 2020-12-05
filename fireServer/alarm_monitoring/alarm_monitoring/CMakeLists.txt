# First attempt at writing my own cmakelists. potentially written badly

# names are still set to gateway related terms because this was originally going be on every gateway instead of as a server for the TTN network.
cmake_minimum_required (VERSION 3.13)

if(UNIX AND NOT APPLE)
    set(LINUX TRUE)
endif()

message("Building project files")
# Add all source files to the GateLib library
add_library(MonitorLib STATIC monitoring.h)

# Link libraries

add_subdirectory(headers)
add_subdirectory(src)


target_include_directories(MonitorLib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})


message("Building 3rd party")
cmake_policy(SET CMP0048 OLD) # does nothing :/ just ignore 0048 I don't want to edit the 3rd party cmake files.
add_subdirectory(lib)

# Please let me use c++17 ...
target_compile_features(MonitorLib PUBLIC cxx_std_17)


# Add source to this project's executable.
add_executable (${PROJECT_NAME} main.cpp)


if (CMAKE_CXX_COMPILER_ID MATCHES "GNU")
	# GCC does not link standard libary features by default...
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-psabi")
	target_link_libraries(${PROJECT_NAME} PRIVATE stdc++fs MonitorLib)
else()
	# add the gateway library to main
	target_link_libraries(${PROJECT_NAME} PRIVATE MonitorLib)
endif()
