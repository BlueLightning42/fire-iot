# CMakeList.txt : Top-level CMake project file, do global configuration
# and include sub-projects here.

cmake_minimum_required (VERSION 3.8)

project ("alarm_monitoring" 
				VERSION 0.1 
				DESCRIPTION "Long running application to be used as the logic behind the alarm monitoring software. fireiot project for capstone 2020")

# please give me c++17 ...
set(CMAKE_CXX_STANDARD 17) 
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Include sub-projects.
add_subdirectory ("alarm_monitoring")
