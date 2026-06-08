// NetGraph.h

#ifndef NETGRAPH_H

#define NETGRAPH_H

#include "Group.h"

class NetGraph {
public:
  NetGraph(Group const &root);
  ~NetGraph();
  QSet<NodeID> net(NodeID seed) const;
  QList<QSet<NodeID>> allNets() const;
  Nodename someNodename(QSet<NodeID> const &net, NodeID seed=NodeID()) const;
  QSet<NodeID> dangling() const;
  QSet<NodeID> adjacent(NodeID seed) const;
private:
  Group const &root;
  class NetGraphData *d;
private:
  NetGraph(NetGraph const &) = delete;
  NetGraph &operator=(NetGraph const &) = delete;
};

#endif
