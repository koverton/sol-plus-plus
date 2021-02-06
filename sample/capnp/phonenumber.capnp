@0xb1033065d1527f09;

using Cxx = import "/capnp/c++.capnp";
$Cxx.namespace( "kov::capnp" );

struct PhoneNumber {
    number @0 :Text;
    type @1 :Type;
    enum Type {
      mobile @0;
      home @1;
      work @2;
    }
}

