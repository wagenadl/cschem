// IDFactory.cpp

#include "IDFactory.h"

IDFactory &IDFactory::instance() {
  static IDFactory factory;
  return factory;
}

IDFactory::IDFactory() {
  max = 0;
}

int IDFactory::newId() {
  max ++;
  return max;
}

void IDFactory::reserve(int id) {
  if (id > max)
    max = id;
}
