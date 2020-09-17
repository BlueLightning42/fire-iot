# CMakeList.txt : Top-level CMake project file, do global configuration
# and include sub-projects here.
#
cmake_minimum_required (VERSION 3.8)

project ("GatewayRPI" 
				VERSION 0.1 
				DESCRIPTION "Gateway for Fire alarm IOT project")

# Include sub-projects.
add_subdirectory ("GatewayRPI")
