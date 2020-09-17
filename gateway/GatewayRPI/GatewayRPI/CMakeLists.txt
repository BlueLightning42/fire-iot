# First attempt at writing my own cmakelists. potentially written badly

cmake_minimum_required (VERSION 3.13)


# Include External librarys
find_package(fmt CONFIG REQUIRED)
find_package(sqlite3 CONFIG REQUIRED)

add_library(GateLib STATIC GatewayRPI.h)

# Link libraries
target_link_libraries(GateLib PRIVATE sqlite3)
target_link_libraries(GateLib PUBLIC fmt::fmt fmt::fmt-header-only)

add_subdirectory(headers)
add_subdirectory(src)

target_include_directories(GateLib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

# Add source to this project's executable.
add_executable (${PROJECT_NAME} main.cpp)

target_link_libraries(${PROJECT_NAME} PRIVATE GateLib)
#target_compile_features(main PUBLIC cxx_std_11)

