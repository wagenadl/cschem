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

bool TraceRepair::fixAllPinTouchings() {
  bool got = false;
  while (true) {
    bool any = false;
    for (int id: d->grp.keys()) {
      Object const &obj(d->grp.object(id));
      if (obj.isTrace()) {
        if (fixPinTouchings(id)) {
          any = true;
          break;
        }
      }
    }
    if (any) {
      got = true;
    } else {
      break;
    }
  }
  return got;
}

bool TraceRepair::fixAllTraceIntersections(Dim grid) {
  bool got = false;
  while (true) {
    bool any = false;
    for (int id: d->grp.keys()) {
      Object const &obj(d->grp.object(id));
      if (obj.isTrace()) {
        if (fixTraceIntersections(id, grid)) {
          any = true;
          break;
        }
      }
    }
    if (any) {
      got = true;
    } else {
      break;
    }
  }
  return got;
}

bool TraceRepair::fixTraceIntersections(int trid, Dim grid) {
  bool got = false;
  Object &obj(d->grp.object(trid));
  if (!obj.isTrace()) {
    qDebug() << "fixPinTouchings: not a trace";
    return got;
  }

  Trace &trace(obj.asTrace());
  while (true) {
    Intersection isec(d->grp, trid);
    Intersection::Result res = isec.touchingTrace(false);
    if (res.node.isEmpty())
      return got;
    
    // So we are touching a trace somewhere along our middle, or their middle.
    // Let's split our trace up, or the other one, or both.
    got = true;
    if (res.point!=trace.p1 && res.point!=trace.p2) {
      // must split us up
      Trace t1(trace);
      t1.p1 = res.point;
      trace.p2 = res.point;
      int id1 = d->grp.insert(Object(t1));
      fixTraceIntersections(id1, grid);
    }
    Trace &trc1(d->grp.object(res.node).asTrace());
    if (res.point!=trc1.p1 && res.point!=trc1.p2) {
      // must split other up
      Trace t1(trc1);
      t1.p1 = res.point;
      trc1.p2 = res.point;
      int id1 = d->grp.insert(Object(t1));
      fixTraceIntersections(id1, grid);
      fixTraceIntersections(res.node.first(), grid);
    }
  }
}
 
bool TraceRepair::fixPinTouchings(int trid) {
  bool got = false;
  Object &obj(d->grp.object(trid));
  if (!obj.isTrace()) {
    qDebug() << "fixPinTouchings: not a trace";
    return got;
  }

  Trace &trace(obj.asTrace());
  while (true) {
    Intersection isec(d->grp, trid);
    Intersection::Result res = isec.touchingPin(false);
    if (res.node.isEmpty())
      return got;
    
    // So we are touching a pin. Let's split our trace up.
    got = true;
    Trace t1(trace);
    t1.p1 = res.point;
    trace.p2 = res.point;
    int id1 = d->grp.insert(Object(t1));
    fixPinTouchings(id1);
  }
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
    
