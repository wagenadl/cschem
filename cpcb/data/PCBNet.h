// PCBNet.h

#ifndef PCBNET_H

#define PCBNET_H

#include "NodeID.h"
#include "Nodename.h"
#include "Group.h"
#include <QSet>

class PCBNet {
public:
  PCBNet(Group const &root, NodeID seed);
  PCBNet();
  QSet<NodeID> const &nodes() const { return nodes_; }
  NodeID seed() const { return seed_; }
  Group const &root() const { return root_; }
  Nodename someNode() const; // a named node on the net, ideally the seed
  void report() const;
private:
  Group root_;
  NodeID seed_;
  QSet<NodeID> nodes_;
  mutable bool havesomenode;
  mutable Nodename somenode;
};

#endif
