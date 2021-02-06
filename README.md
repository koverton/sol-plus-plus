# sol-plus-plus
Lightweight modern c++ wrapper for Solace messaging client

This is not a full library, just a client CPP/HPP pair that can be dropped into a project for quick connectivity. Initial version is just for testing.

The solclient CCSMP library is a free download a included for easy building; it can be swapped for newer versions via their download site https://solace.com/downloads/

## Build `sample/subscriber.cpp`

```bash
bash% mkdir Debug
bash% cd Debug
bash% cmake ..
-- The C compiler identification is AppleClang 12.0.0.12000032
-- The CXX compiler identification is AppleClang 12.0.0.12000032
...
...
-- Configuring done
-- Generating done
-- Build files have been written to: .../sol-plus-plus/Debug
bash% make
...
...
[100%] Built target subscriber
bash% ls -l subscriber
-rwxr-xr-x  1 kov  staff  211256 Jan 10 09:45 subscriber*
```

## Overview

Designed for ease of use in c++11 and higher builds. For example, you can use lambdas for callbacks as follows:

```c++
#include "solace_client.hpp"
#include <string>
#include <iostream>
#include <ctime>
#include <chrono>

int main(int c, char** a)
{
	using namespace kov::solace;
	using State   = kov::solace::SolClient::State;
	using MsgInfo = kov::solace::SolClient::MsgInfo;

	std::string topic{"test/topic"};
	// Create a default config, then override where necessary
	SolConfig config;
	config._host = "broker.solace.com";
	config._port = 54321;
	// Create the client based on config and connect
	SolClient client{config};
	bool isconnected{false};
	client.connect(
		// Connectivity event handler
		[&](State state)
		{
			if (state == State::CONNECTED)
				client.subscribe(topic);
		},
		// Message event handler
		[&](const MsgInfo& e, const void* d, const size_t l)
		{
			// DEMO ONLY! Requires sender actually 
			// sends string content in the binary payload
			std::string_view msg{static_cast<const char*>(d), l};
			std::cout << "SUBSCRIBER MESSAGE: " << msg << std::endl;
			std::cout << "  topic: " << e.topic << std::endl;
			std::cout << "  msgid: " << e.msgid << std::endl;
			auto rt = std::chrono::system_clock::to_time_t(e.rcvtime);
			std::cout << "  rcvtm: " << std::ctime(&rt);
			auto st = std::chrono::system_clock::to_time_t(e.sndtime);
			std::cout << "  sndtm: " << std::ctime(&st);
		}
	);
	// ...
}
```

