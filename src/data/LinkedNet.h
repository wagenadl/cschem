// LinkedNet.h

#ifndef LINKEDNET_H

#define LINKEDNET_H

#include "circuit/Schem.h"
#include "circuit/Circuit.h"
#include "circuit/Net.h"
#include "Nodename.h"

class LinkedNet {
public:
  LinkedNet(Circuit const &circ, Net const &net);
  LinkedNet() { }
  bool containsMatch(Nodename const &) const;
  QString name;
  QList<Nodename> nodes;
  void report();
};

#endif
