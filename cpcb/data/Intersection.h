// Intersection.h

#ifndef INTERSECTION_H

#define INTERSECTION_H

#include "Group.h"
#include "Trace.h"
#include "NodeID.h"
#include "NetGraph.h"

class Intersection {
public:
  struct Result {
  public:
    NodeID node;
    Point point;
  public:
    Result() {}
    Result(NodeID nid, Point p): node(nid), point(p) {}
  };
public:
  Intersection(NetGraph const &graph, NodeID trcid);
  QList<Result> touchingPins(bool allowends) const; // goes into groups
  QList<Result> touchingTraces(bool allowends) const; // does not go into groups
  QList<NodeID> fullyContained() const; // traces fully contained within us
private:
  NetGraph const &graph;
  NodeID traceid; // id of trace; cannot be returned as a touching trace.
};

#endif
