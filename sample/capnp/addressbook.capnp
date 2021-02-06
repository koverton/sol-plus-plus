@0xe1ad9fa43491e15a;

using Cxx = import "/capnp/c++.capnp";
$Cxx.namespace( "kov::capnp" );

using import "person.capnp".Person;

struct AddressBook {
  people @0 :List(Person);
}

