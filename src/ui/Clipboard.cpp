// Clipboard.cpp

#include "Clipboard.h"
#include "circuit/CircuitMod.h"
Clipboard::Clipboard() {
}

Clipboard &Clipboard::clipboard() {
  static Clipboard cb;
  return cb;
}

void Clipboard::store(Circuit const &c, SymbolLibrary const &l) {
  circ = c;
  lib = l;
  CircuitMod cm(circ, lib);
  for (int c: circ.elements.keys())
    cm.removePointlessJunction(c);
  circ = cm.circuit();
}

Circuit Clipboard::retrieve() const {
  return circ;
}

SymbolLibrary const &Clipboard::library() const {
  return lib;
}
