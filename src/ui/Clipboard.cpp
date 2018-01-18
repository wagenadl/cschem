// Clipboard.cpp

#include "Clipboard.h"

Clipboard::Clipboard() {
}

Clipboard &Clipboard::clipboard() {
  static Clipboard cb;
  return cb;
}

void Clipboard::store(Circuit const &c) {
  circ = c;
}

Circuit Clipboard::retrieve() const {
  return circ;
}

