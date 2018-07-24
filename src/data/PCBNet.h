// PCBNet.h

#ifndef PCBNET_H

#define PCBNET_H

#include "NodeID.h"
#include <QSet>

class PCBNet {
public:
  PCBNet(class Group const &root, NodeID seed);
  QSet<NodeID> net() const { return net_; }
private:
  QSet<NodeID> net_;
};

#endif
