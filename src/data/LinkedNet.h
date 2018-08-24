// LinkedNet.h

#ifndef LINKEDNET_H

#define LINKEDNET_H

#include "circuit/Schem.h"
#include "circuit/Circuit.h"
#include "circuit/Net.h"
#include "Nodename.h"

class LinkedNet {
public:
  LinkedNet(Schem const &schem, Net const &net);
  LinkedNet() { }
  bool containsMatch(Nodename const &) const;
public:
  QString name;
  QList<Nodename> nodes;
};

QDebug &operator<<(QDebug &, LinkedNet const &);

#endif
