cmake_minimum_required(VERSION 3.10)

# set the project name
project(sol++ VERSION 0.0.1 DESCRIPTION "C++ wrapper for Solace event client")

set(SOLACE_DIR ${PROJECT_SOURCE_DIR}/solclient-7.15.0.47)
include(${PROJECT_SOURCE_DIR}/options.cmake)


include_directories(${PROJECT_SOURCE_DIR}/include ${SOLACE_DIR}/include)
link_directories(${SOLACE_DIR}/lib)

SET(SOLACE_FLAGS "-DSOLCLIENT_CONST_PROPERTIES -DPROVIDE_LOG_UTILITIES")
add_definitions(${SOLACE_FLAGS})


# add the executable
add_executable(subscriber 
	src/solace_client.cpp
	sample/subscriber.cpp)

target_link_libraries(subscriber solclient)
