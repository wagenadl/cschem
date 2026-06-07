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
private:
  NetGraph(NetGraph const &) = delete;
  NetGraph &operator=(NetGraph const &) = delete;
  class NetGraphData *d;
};

#endif
