@0x8ebfdc1a6523c928;

using Cxx = import "/capnp/c++.capnp";
$Cxx.namespace( "kov::capnp" );

using import "phonenumber.capnp".PhoneNumber;

struct Person {
  id @0 :UInt32;
  name @1 :Text;
  email @2 :Text;
  phones @3 :List(PhoneNumber);
  employment :union {
    unemployed @4 :Void;
    employer @5 :Text;
    school @6 :Text;
    selfEmployed @7 :Void;
    # We assume that a person is only one of these.
  }
}

