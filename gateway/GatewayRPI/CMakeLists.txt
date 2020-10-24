# CMakeList.txt : Top-level CMake project file, do global configuration
# and include sub-projects here.

cmake_minimum_required (VERSION 3.8)

project ("GatewayRPI" 
				VERSION 0.1 
				DESCRIPTION "Gateway for Fire alarm IOT project")

# please give me c++17 ...
set(CMAKE_CXX_STANDARD 17) 
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Include sub-projects.
add_subdirectory ("GatewayRPI")
