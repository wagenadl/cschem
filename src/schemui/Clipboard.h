// Clipboard.h

#ifndef CLIPBOARD_H

#define CLIPBOARD_H

#include "circuit/Circuit.h"

class Clipboard {
public:
  Clipboard();
  static Clipboard &clipboard();
  void store(Circuit const &);
  Circuit retrieve() const;
private:
  Circuit circ;
};

#endif
