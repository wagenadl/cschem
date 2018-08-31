// Intersection.h

#ifndef INTERSECTION_H

#define INTERSECTION_H

#include "Group.h"
#include "Trace.h"
#include "NodeID.h"

class Intersection {
public:
  struct Result {
  public:
    NodeID node; // empty for invalid, relative to group
    Point point;
  };
public:
  Intersection(Group const &group, Trace const &trace, bool allowends);
  Result touchingPin() const;
  Result touchingTrace() const;
private:
  Group const &group;
  Trace const &trace;
  bool allowends;
};

#endif
