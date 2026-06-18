// Intersection.cpp

#include "Intersection.h"
#include "Object.h"

Intersection::Intersection(NetGraph const &graph, NodeID traceid):
  graph(graph), traceid(traceid) {
}

QList<Intersection::Result> Intersection::touchingPins(bool allowends) const {
  Group const &group(graph.root());
  QList<Intersection::Result> res;
  if (!group.object(traceid).isTrace()) {
    qDebug() << "Not a trace" << traceid;
    return res;
  }
  Trace const &trace(group.object(traceid).asTrace());
  QSet<NodeID> adj(graph.adjacent(traceid));
  for (NodeID const &nid: adj) {
    Object const &obj(group.object(nid));
    switch (obj.type()) {
    case Object::Type::Hole: {
      Hole const &hole = obj.asHole();
      if (allowends || (hole.p != trace.p1 && hole.p != trace.p2))
        res << Result(nid, hole.p);
    } break;
    case Object::Type::Pad: {
      Pad const &pad = obj.asPad();
      if (allowends || (pad.p != trace.p1 && pad.p != trace.p2))
        res << Result(nid, pad.p);
    } break;      
    default:
      break;
    }
  }
  return res;
}

QList<NodeID> Intersection::fullyContained() const {
  Group const &group(graph.root());
  QList<NodeID> res;
  if (!group.object(traceid).isTrace()) {
    qDebug() << "Not a trace" << traceid;
    return res;
  }
  Trace const &trace(group.object(traceid).asTrace());
  QSet<NodeID> adj(graph.adjacent(traceid));
  for (NodeID const &nid: adj) {
    if (nid.size() != 1)
      continue; // we do not go into groups
    Object const &obj(group.object(nid));
    if (!obj.isTrace())
      continue;
    Trace const &other(obj.asTrace());
    if (trace.projectsWithin(other.p1, Dim::fromMils(1))
        && trace.projectsWithin(other.p2, Dim::fromMils(1)))
      res << nid;
  }
  return res;
}

QList<Intersection::Result> Intersection::touchingTraces(bool allowends) const {
  Group const &group(graph.root());
  QList<Intersection::Result> res;
  if (!group.object(traceid).isTrace()) {
    qDebug() << "Not a trace" << traceid;
    return res;
  }
  Trace const &trace(group.object(traceid).asTrace());
  QSet<NodeID> adj(graph.adjacent(traceid));
  for (NodeID const &nid: adj) {
    if (nid.size() != 1)
      continue; // we do not go into groups
    Object const &obj(group.object(nid));
    switch (obj.type()) {
    case Object::Type::Trace: {
      Trace const &tr1(obj.asTrace());
      std::optional<Point> op(tr1.touchPoint(trace));
      if (!op) {
        qDebug() << "Missing intersection";
        break;
      }
      if (allowends
          || (*op != tr1.p1 && *op != tr1.p2)
          || (*op != trace.p1 && *op != trace.p2)) 
        res << Result(nid, *op);
    } break;
    default:
      break;
    }
  }
  return res;
}
