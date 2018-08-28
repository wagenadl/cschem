// NodeID.h

#ifndef NODEID_H

#define NODEID_H

#include <QVector>
#include "VectorCf.h"
#include "LayerPoint.h"

class NodeID: public QVector<int> {
public:
  bool operator==(NodeID const &x) const { return vectorEq(*this, x); }
  bool operator<(NodeID const &x) const { return vectorLt(*this, x); }
  class Object const &deref(class Group const &root) const;
  LayerPoint location(class Group const &root, bool *ok=0) const;
  NodeID plus(int x) const { NodeID res(*this); res << x; return res; }
  NodeID tail() const { NodeID res(*this); res.removeFirst(); return res; }
};

inline uint qHash(NodeID const &id) {
  uint hsh = 0;
  for (int bit: id) {
    hsh = 2346781*hsh ^ bit;
  }
  return hsh;
}

#endif
