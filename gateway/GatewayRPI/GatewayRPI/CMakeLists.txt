# First attempt at writing my own cmakelists. potentially written badly

cmake_minimum_required (VERSION 3.8)


# Include External librarys
find_package(fmt CONFIG REQUIRED)
find_package(sqlite3 CONFIG REQUIRED)




# Add source files
add_library(gateway
    ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/Communication.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/Gateway.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/Storage.cpp
)
# Add header files
target_include_directories (gateway 
	PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/headers
)
# Add source to this project's executable.
add_executable (${PROJECT_NAME} gateway)


#target_compile_features(main PUBLIC cxx_std_11)

# Link libraries
target_link_libraries(${PROJECT_NAME} PRIVATE sqlite3)
target_link_libraries(${PROJECT_NAME} PRIVATE fmt::fmt fmt::fmt-header-only)