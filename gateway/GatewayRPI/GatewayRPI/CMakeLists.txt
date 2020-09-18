# First attempt at writing my own cmakelists. potentially written badly

cmake_minimum_required (VERSION 3.13)


# Include External librarys
find_package(fmt CONFIG REQUIRED)
find_package(unofficial-sqlite3 CONFIG REQUIRED)

# Add all source files to the GateLib library
add_library(GateLib STATIC GatewayRPI.h)

# Link libraries

add_subdirectory(headers)
add_subdirectory(src)

target_link_libraries(GateLib PRIVATE unofficial::sqlite3::sqlite3)
target_link_libraries(GateLib PUBLIC fmt::fmt fmt::fmt-header-only)


target_include_directories(GateLib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

# Please let me use c++11
target_compile_features(GateLib PUBLIC cxx_std_11)

# Add source to this project's executable.
add_executable (${PROJECT_NAME} main.cpp)

target_link_libraries(${PROJECT_NAME} PRIVATE GateLib)
