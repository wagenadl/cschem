// NetMismatch.h

#ifndef NETMISMATCH_H

#define NETMISMATCH_H

#include <QSet>
#include "NodeID.h"
#include "Nodename.h"

class NetMismatch {
public:
  NetMismatch();
  void reset();
  void recalculate(class PCBNet const &net, class LinkedNet const &linkednet,
		   class Group const &root);
  void report(Group const &root);
public:
  QSet<NodeID> wronglyInNet;
  QSet<NodeID> missingFromNet;
  QSet<Nodename> missingEntirely;
};

#endif
