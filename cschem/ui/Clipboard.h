// Clipboard.h

#ifndef CLIPBOARD_H

#define CLIPBOARD_H

#include "circuit/Circuit.h"
#include "svg/SymbolLibrary.h"

class Clipboard {
public:
  Clipboard();
  static Clipboard &clipboard();
  void store(Circuit const &, SymbolLibrary const &);
  Circuit retrieve() const;
  SymbolLibrary const &library() const;
private:
  Circuit circ;
  SymbolLibrary lib;
};

#endif
