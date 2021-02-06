cmake_minimum_required(VERSION 3.10)

# set the project name
project(sol++ VERSION 0.0.1 DESCRIPTION "C++ wrapper for Solace event client")

set( SAMPLE_DIR ${PROJECT_SOURCE_DIR}/sample )
set( CPP_SRC_DIR ${SAMPLE_DIR}/cpp )

set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DCAPNP_ALLOW_UNALIGNED" )

#
# Capnp setup
#
find_package(CapnProto CONFIG REQUIRED )
set( CAPNP_SRC_DIR ${SAMPLE_DIR}/capnp )
capnp_generate_cpp( capnp_srcs capnp_hdrs 
	${CAPNP_SRC_DIR}/addressbook.capnp
	${CAPNP_SRC_DIR}/person.capnp
	${CAPNP_SRC_DIR}/phonenumber.capnp )
include_directories(  ${CMAKE_CURRENT_BINARY_DIR}/sample/capnp )

#
# Solace setup
#
set( SOLACE_DIR ${PROJECT_SOURCE_DIR}/solclient-7.15.0.47 )
include( ${PROJECT_SOURCE_DIR}/options.cmake )
include_directories( ${PROJECT_SOURCE_DIR}/include ${SOLACE_DIR}/include )
link_directories( ${SOLACE_DIR}/lib )
SET( SOLACE_FLAGS "-DSOLCLIENT_CONST_PROPERTIES -DPROVIDE_LOG_UTILITIES" )
add_definitions( ${SOLACE_FLAGS} )


# add the executable
add_executable( pubsub 
	${PROJECT_SOURCE_DIR}/sample/cpp/pubsub.cpp
	${PROJECT_SOURCE_DIR}/src/solace_client.cpp 
	${capnp_srcs} )

target_include_directories( pubsub PRIVATE ${CMAKE_CURRENT_BINARY_DIR} )
target_link_libraries (     pubsub PRIVATE solclient CapnProto::capnp )
