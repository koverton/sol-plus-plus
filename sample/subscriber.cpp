#include "solace_client.hpp"
#include <string>
#include <string_view>
#include <iostream>
#include <thread>
#include <ctime>
#include <chrono>


int main(int c, char** a) 
{
	using namespace kov::solace;
	using State   = kov::solace::SolClient::State;

	std::string topic{"test/topic"};

	SolConfig config; // sets default variables
	config._host = "broker.solace.com"; // custom host
	config._port = 55014; // custom port, etc.
	SolClient client{config};
	bool isconnected{false};
	client.connect(
		[&](State state)
		{
			isconnected = (state == State::CONNECTED);
		},
		[&](const Envelope& e, const void* d, const size_t l)
		{
			// Dangerous! Assume message is a string
			std::string_view msg{static_cast<const char*>(d), l};
			std::cout << "SUBSCRIBER MESSAGE: " << msg << std::endl;
			std::cout << "	topic: " << e.topic << std::endl;
			std::cout << "	msgid: " << e.msgid << std::endl;
			auto rt = std::chrono::system_clock::to_time_t(e.rcvtime);
			std::cout << "	rcvtm: " << std::ctime(&rt);
			auto st = std::chrono::system_clock::to_time_t(e.sndtime);
			std::cout << "	sndtm: " << std::ctime(&st);
		}
	);

	while(!isconnected) 
		std::this_thread::sleep_for( std::chrono::seconds{1} );

	client.subscribe(topic);

	while(true) 
		std::this_thread::sleep_for( std::chrono::seconds{1} );

	return 0;
}

