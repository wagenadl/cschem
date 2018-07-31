// PCBNet.h

#ifndef PCBNET_H

#define PCBNET_H

#include "NodeID.h"
#include <QSet>

class PCBNet: public QSet<NodeID> {
public:
  PCBNet(class Group const &root, NodeID seed);
  PCBNet() {}
};

#endif
