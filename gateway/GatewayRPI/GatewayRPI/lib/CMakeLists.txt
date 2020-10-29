
cmake_minimum_required (VERSION 3.13)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/fmt/include)

# fmt header only library
# add_library(fmt INTERFACE)
# target_compile_definitions(fmt INTERFACE FMT_HEADER_ONLY=1)
# target_include_directories(fmt INTERFACE
#  ${CMAKE_CURRENT_SOURCE_DIR}/fmt/include)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/fmt)
target_link_libraries(GateLib fmt)

# same config as https://github.com/SRombauts/SQLiteCpp_Example/blob/master/CMakeLists.txt
set(SQLITECPP_RUN_CPPCHECK OFF CACHE BOOL "" FORCE)
set(SQLITECPP_RUN_CPPLINT OFF CACHE BOOL "" FORCE)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/SQLite)

# Link gatelib with with SQLiteCpp and fmt
target_link_libraries(GateLib SQLiteCpp)


if(LINUX) # throws errors from all the linux specific code when I test in Visual studio...runs as normal on the pi
	# add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/MQTT_C) # dependancy for the c++ mqtt
	# add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/MQTT_CPP)
	set(RadioHeadDir ${CMAKE_CURRENT_SOURCE_DIR}/RadioHead_fork/)
	include_directories(${RadioHeadDir}/)

	add_library(RadioHead)
	target_compile_definitions(RadioHead PUBLIC
		-Dsimulator_h # stop error: redefinition of ‘class SerialSimulator’s
		-DRASPBERRY_PI 
		-DBCM2835_NO_DELAY_COMPATIBILITY
		-Wreturn-type
	)
	target_include_directories(RadioHead PUBLIC 
		${RadioHeadDir}/
	)
	# all the sources in gateway\GatewayRPI\GatewayRPI\lib\RadioHead_fork\examples\raspi\rf95\Makefile
	target_sources(RadioHead PUBLIC
    	${RadioHeadDir}/RH_RF95.h
    	${RadioHeadDir}/RH_RF95.cpp
    	${RadioHeadDir}/RHDatagram.cpp
    	${RadioHeadDir}/RHHardwareSPI.cpp
    	${RadioHeadDir}/RHSPIDriver.cpp
    	${RadioHeadDir}/RHGenericSPI.cpp
    	${RadioHeadDir}/RHGenericDriver.cpp
    	${RadioHeadDir}/RHutil/RasPi.h
    	${RadioHeadDir}/RHutil/RasPi.cpp
	)

	list(APPEND CMAKE_PREFIX_PATH ${CMAKE_CURRENT_SOURCE_DIR}/lib/bcm283/src)
	
	target_link_libraries(RadioHead bcm2835)

	target_include_directories(RadioHead PUBLIC ${RadioHeadDir}/)

	target_link_libraries(GateLib RadioHead)

endif()
