// foo.cpp

#include "foo.h"

class foodata: public QSharedData {
public:
  foodata() { x=0;
  }
public:
  int x;
};

foo::foo() {
  d = new foodata;
}

foo::~foo() {
}

foo::foo(foo const &f) {
  d = f.d;
}

foo &foo::operator=(foo const &f) {
  d = f.d;
  return *this;
}

int &foo::x() {
  return d->x;
}

