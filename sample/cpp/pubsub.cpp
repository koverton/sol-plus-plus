#include "addressbook.capnp.h"

#include <solace_client.hpp>
#include <capnp/message.h>
#include <capnp/serialize-packed.h>

#include <string>
#include <string_view>
#include <iostream>
#include <thread>
#include <ctime>
#include <chrono>

using kov::capnp::AddressBook;
using kov::capnp::Person;
using kov::capnp::PhoneNumber;

void
writeAddressBook( ::capnp::MallocMessageBuilder& message) {
  AddressBook::Builder addressBook = message.initRoot<AddressBook>();
  ::capnp::List<Person>::Builder people = addressBook.initPeople(2);

  Person::Builder alice = people[0];
  alice.setId(123);
  alice.setName("Alice");
  alice.setEmail("alice@example.com");
  // Type shown for explanation purposes; normally you'd use auto.
  ::capnp::List<PhoneNumber>::Builder alicePhones =
      alice.initPhones(1);
  alicePhones[0].setNumber("555-1212");
  alicePhones[0].setType(PhoneNumber::Type::MOBILE);
  alice.getEmployment().setSchool("MIT");

  Person::Builder bob = people[1];
  bob.setId(456);
  bob.setName("Bob");
  bob.setEmail("bob@example.com");
  auto bobPhones = bob.initPhones(2);
  bobPhones[0].setNumber("555-4567");
  bobPhones[0].setType(PhoneNumber::Type::HOME);
  bobPhones[1].setNumber("555-7654");
  bobPhones[1].setType(PhoneNumber::Type::WORK);
  bob.getEmployment().setUnemployed();
}

void 
printAddressBook(const void* buf, uint32_t length) {
  const ::capnp::word* const words = static_cast<const ::capnp::word*>(buf);
  ::capnp::FlatArrayMessageReader message({words, length});

  AddressBook::Reader addressBook = message.getRoot<AddressBook>();

  for (Person::Reader person : addressBook.getPeople()) {
    std::cout << person.getName().cStr() << ": "
              << person.getEmail().cStr() << std::endl;
    for (PhoneNumber::Reader phone: person.getPhones()) {
      const char* typeName = "UNKNOWN";
      switch (phone.getType()) {
        case PhoneNumber::Type::MOBILE: typeName = "mobile"; break;
        case PhoneNumber::Type::HOME: typeName = "home"; break;
        case PhoneNumber::Type::WORK: typeName = "work"; break;
      }
      std::cout << "  " << typeName << " phone: "
                << phone.getNumber().cStr() << std::endl;
    }
    Person::Employment::Reader employment = person.getEmployment();
    switch (employment.which()) {
      case Person::Employment::UNEMPLOYED:
        std::cout << "  unemployed" << std::endl;
        break;
      case Person::Employment::EMPLOYER:
        std::cout << "  employer: "
                  << employment.getEmployer().cStr() << std::endl;
        break;
      case Person::Employment::SCHOOL:
        std::cout << "  student at: "
                  << employment.getSchool().cStr() << std::endl;
        break;
      case Person::Employment::SELF_EMPLOYED:
        std::cout << "  self-employed" << std::endl;
        break;
    }
  }
}

int main(int c, char** a) 
{
	using namespace std::chrono;
	using namespace kov::solace;
	using State   = kov::solace::SolClient::State;
	using MsgInfo = kov::solace::SolClient::MsgInfo;

	std::string topic{"test/topic"};
  std::string smsg{"Hello world!"};

	SolConfig config; // sets default variables
	config._host = "localhost"; // custom host
	config._port = 55020; // custom port, etc.
	SolClient client{config};
	bool isconnected{false};
	bool msgreceived{false};
	client.connect(
		[&](State state)
		{
			isconnected = (state == State::CONNECTED);
		},
		[&](const MsgInfo& e, const void* d, const size_t l)
		{      
			std::cout << "	topic: " << e.topic << std::endl;
			std::cout << "	msgid: " << e.msgid << std::endl;
			auto rt = system_clock::to_time_t(e.rcvtime);
			std::cout << "	rcvtm: " << std::ctime(&rt);
			auto st = system_clock::to_time_t(e.sndtime);
			std::cout << "	sndtm: " << std::ctime(&st) << std::endl;
      std::cout << "  payload size: " << l << std::endl;

			// Dangerous! Assume message is a string
			// std::string_view msg{static_cast<const char*>(d), l};
			// std::cout << "SUBSCRIBER MESSAGE: " << msg << std::endl;
      printAddressBook( d, (uint32_t)l );

      msgreceived = true;
		}
	);

	while(!isconnected) 
		std::this_thread::sleep_for( seconds{1} );

  std::cout << "Subscribing to " << topic << std::endl;
	client.subscribe(topic);

  ::capnp::MallocMessageBuilder message;
  writeAddressBook( message );
  const kj::Array<const capnp::word> buf = ::capnp::messageToFlatArray(message);
  const kj::ArrayPtr<const capnp::byte> bytes = buf.asBytes();
    printAddressBook( bytes.begin(), (uint32_t)bytes.size() );
      std::cout << "  payload size: " << bytes.size() << std::endl;


  // std::cout << "Publishing to " << topic << std::endl;
  // client.publish( topic, smsg.c_str(), (uint32_t)smsg.size() );
  client.publish( topic, bytes.begin(), (uint32_t)bytes.size() );


	while(!msgreceived) 
  {
    std::cout << "Waiting..." << std::endl;
		std::this_thread::sleep_for( seconds{1} );
  }

	return 0;
}

