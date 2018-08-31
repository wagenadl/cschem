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
  Intersection(Group const &group, int trcid);
  Intersection(Group const &group, Trace const &trace);
  Result touchingPin(bool allowends) const; // goes into groups
  Result touchingTrace(bool allowends) const; // does not go into groups
private:
  Group const &group;
  Trace const &trace;
  int traceid; // id of trace; cannot be returned as a touching trace.
};

#endif
