// TraceRepair.cpp

#include "TraceRepair.h"
#include "Group.h"
#include "Intersection.h"
#include "Object.h"
#include "NetGraph.h"
#include "TicToc.h"

class RepairData {
public:
  RepairData(Group &grp): grp(grp) { }
public:
  Group &grp;
};

TraceRepair::TraceRepair(Group &grp): d(new RepairData(grp)) {
}

TraceRepair::~TraceRepair() {
  delete d;
}

bool TraceRepair::fixPinTouchings() {
  bool any = false;
  QList<int> universe = d->grp.keys();
  TicToc tmr;
  while (!universe.isEmpty()) {
    QSet<int> newuniverse;
    NetGraph ng(d->grp);
    qDebug() << "fixpt got graph" << tmr.lap();
    for (int id: universe) {
      if (newuniverse.contains(id))
        continue; // wait for next round, already altered
      Object const &obj(d->grp.object(id));
      if (!obj.isTrace())
        continue;
      Trace const &us(obj.asTrace());
      Intersection isec(ng, NodeID(id));
      for (Intersection::Result const &res: isec.touchingPins(false)) {
        if (!us.onP1(res.point) && !us.onP2(res.point)) {
          Trace &us(d->grp.object(NodeID(id)).asTrace());
          qDebug() << "split us" << us.p1 << res.point << us.p2 << res.point.distance(us.p1) << res.point.distance(us.p2) << us.width/2;
          Trace t1(us);
          us.p2 = res.point;
          t1.p1 = res.point;
          int id1 = d->grp.insert(Object(t1)); // insert new part
          newuniverse << id << id1; // recheck both parts
          break; // only handle one pin at a time for a given trace
        }
      }
    }
    universe = newuniverse.values();
    if (!newuniverse.isEmpty())
      any = true;
  }
  return any;
}

bool TraceRepair::fixTraceIntersections() {
  bool any = false;
  QList<int> universe = d->grp.keys();
  TicToc tmr;
  qDebug() << "looking for contained";
  while (!universe.isEmpty()) {
    QSet<int> deleted;
    NetGraph ng(d->grp);
    qDebug() << "fixtr got graph" << tmr.lap();
    for (int id: universe) {
      if (deleted.contains(id))
        continue; // wait for next round, already altered
      Object const &obj(d->grp.object(id));
      if (!obj.isTrace())
        continue;
      Trace const &us(obj.asTrace());
      Intersection isec(ng, NodeID(id));
      for (NodeID const &res: isec.fullyContained()) {
        deleted << res.first();
        d->grp.remove(res.first());
      }
    }
    universe = d->grp.keys();
    if (deleted.isEmpty())
      break;
  }
  
  qDebug() << "looking for intersections";
  while (!universe.isEmpty()) {
    QSet<int> newuniverse;
    NetGraph ng(d->grp);
    qDebug() << "fixtr got graph" << tmr.lap();
    for (int id: universe) {
      if (newuniverse.contains(id))
        continue; // wait for next round, already altered
      Object const &obj(d->grp.object(id));
      if (!obj.isTrace())
        continue;
      Trace const &us(obj.asTrace());
      Intersection isec(ng, NodeID(id));
      for (Intersection::Result const &res: isec.touchingTraces(false)) {
        if (newuniverse.contains(res.node.first()))
          continue; // wait for next round, already altered
        Trace const &them(d->grp.object(res.node).asTrace());
        if (!us.onP1(res.point) && !us.onP2(res.point)) {
          // must split us up
          Trace &us(d->grp.object(id).asTrace());
          qDebug() << "split us" << us.p1 << res.point << us.p2 << res.point.distance(us.p1) << res.point.distance(us.p2) << us.width/2 << them.p1 << them.p2;
          Trace t1(us); // make a copy
          us.p2 = res.point;
          t1.p1 = res.point;
          int id1 = d->grp.insert(Object(t1)); // insert new part
          newuniverse << id << id1; // recheck both parts
        }
        if (!them.onP1(res.point) && !them.onP2(res.point)) {
          // must split them up
          Trace &them(d->grp.object(res.node).asTrace());
          qDebug() << "split them" << them.p1 << res.point << them.p2 << res.point.distance(them.p1) << res.point.distance(them.p2) << them.width/2 << us.p1 << us.p2;
          Trace t1(them);
          them.p2 = res.point;
          t1.p1 = res.point;
          int id1 = d->grp.insert(Object(t1)); // insert new part
          newuniverse << id << id1; // recheck both parts
        }
      }
    }
    universe = newuniverse.values();
    if (!newuniverse.isEmpty())
      any = true;
  }
  return any;
}
 
bool TraceRepair::dropDanglingTraces() {
  TicToc tmr;

  bool any = false;
  while (true) {
    NetGraph ng(d->grp);
    auto dangling = ng.dangling();
    qDebug() << "dangling" << tmr.lap();

    QSet<int> dropme;
    for (NodeID const &nid: dangling) {
      if (nid.size() > 1)
        continue; // don't drop inside groups
      Object const &obj(d->grp.object(nid));
      switch (obj.type()) {
      case Object::Type::Trace:
        if (layerIsCopper(obj.asTrace().layer))
          dropme << nid.first();
        break;
      case Object::Type::Hole:
        if (obj.asHole().via)
          dropme << nid.first();
        break;
      default:
        break;
      }
    }
    if (dropme.isEmpty())
      break;
    any = true;
    qDebug() << "dropping" << dropme.size() << "items" << tmr.lap();

    for (int id: dropme) 
      d->grp.remove(id);
  }
  qDebug() << "dangledone" << tmr.lap() << tmr.total();
  return any;
}
    
