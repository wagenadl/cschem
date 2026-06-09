// TraceRepair.cpp

#include "TraceRepair.h"
#include "Group.h"
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
  /* If any trace touches a hole or a pad but not at an endpoint, we will either
     move that endpoint if it is already overlapping with the pin, or
     split the trace.

     This is an iterative procedure, and in extreme situations, may
     change the connectivity.  We do not check for that.

     Even holes or pads inside groups count.
  */
  bool any = false;
  QList<int> universe = d->grp.keys();
  TicToc tmr;
  while (!universe.isEmpty()) {
    QSet<int> newuniverse;
    NetGraph ng(d->grp);
    qDebug() << "fixpt got graph" << tmr.lap();
    for (int id: universe) {
      Object const &obj(d->grp.object(id));
      if (!obj.isTrace())
        continue;
      Trace const &us(obj.asTrace());
      for (NodeID const &nid: ng.adjacent(NodeID(id))) {
        Object const &obj1(d->grp.object(nid));
        if (!obj1.isHoleOrPad())
          continue;
        Point p = obj1.point();
        if (p == us.p1 || p == us.p2)
          continue; // already on an endpoint
        if (us.distanceToLine(p) >= Dim::fromMM(0.2)) // too far
          continue;
        if (us.onP1(p)) {
          //qDebug() << "near p1" << us.p1 << us.p2 << us.width << us.p1.distance(us.p2) << ":" << p << us.p1.distance(p);
          Trace &us(d->grp.object(NodeID(id)).asTrace());
          us.p1 = p;
          newuniverse << id;
          break;
        } else if (us.onP2(p)) {
          //qDebug() << "near p2" << us.p1 << us.p2 << us.width << us.p1.distance(us.p2) << ":" << p << us.p2.distance(p);
          Trace &us(d->grp.object(NodeID(id)).asTrace());
          us.p2 = p;
          newuniverse << id;
          break;
        } else if (us.projectsWithin(p)) {
          // pin touches us, but not near endpoint
          //qDebug() << "middle" << us.p1 << us.p2 << us.width << us.p1.distance(us.p2) << ":" << p << us.p1.distance(p) << us.p2.distance(p) << us.distanceToLine(p);
          Trace &us(d->grp.object(NodeID(id)).asTrace());
          Trace t1 = us;
          us.p2 = p;
          t1.p1 = p;
          int id1 = d->grp.insert(Object(t1));
          newuniverse << id << id1;
          break;
        }
      }
    }
    qDebug() << "count" << newuniverse.size() << tmr.lap();
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
  int iter = 0;
  while (!universe.isEmpty()) {
    QSet<int> newuniverse;
    QSet<int> deleted;
    NetGraph ng(d->grp);
    qDebug() << "fixtr got graph" << tmr.lap();
    for (int id: universe) {
      if (newuniverse.contains(id) || deleted.contains(id))
        continue;
      Object const &obj(d->grp.object(id));
      if (!obj.isTrace())
        continue;
      Trace const &us(obj.asTrace());
      for (NodeID const &nid: ng.adjacent(NodeID(id))) {
        if (nid.size() != 1)
          continue;
        if (nid.first() == id) {
          qDebug() << "self";
          continue;
        }
        if (newuniverse.contains(nid.first()) || deleted.contains(nid.first()))
          continue;
        Object const &obj1(d->grp.object(nid));
        if (!obj1.isTrace())
          continue;
        Trace const &them(obj1.asTrace());
        /* We have to consider collinear and crossing separately
           (1) Collinear
               Using [ and ] to indicate our end points and ( and ) for theirs, we have to consider two cases
                 (a) [--------(=====)------]
                 (b) [--------(=======]--------)
               For (1a), if they are thicker than us, we split, else they are dropped.
               For (1b), if they are thicker than us, our end point is moved, else theirs.
           (2) Crossing
               By construction, the intersection point lies on both lines
               If intersection point within us but not near end point, we split
               If intersection point within them but not near end point, they split
           (3) Parallel but not too close
               Ignore
        */
        std::optional<Point> isec(us.intersection(them));
        if (isec) {
          // crossing
          if (them.betweenEndpoints(*isec, them.width/2)) {
            // they split
            Trace &them(d->grp.object(nid).asTrace());
            qDebug() << "they split" << us.p1 << us.p2 << "|" << *isec << "|" << them.p1 << them.p2;
            Trace t1 = them;
            them.p2 = *isec;
            t1.p1 = *isec;
            int id1 = d->grp.insert(Object(t1));
            newuniverse << id << id1;
          }
          if (us.betweenEndpoints(*isec, us.width/2)) {
            // we split
            Trace &us(d->grp.object(NodeID(id)).asTrace());
            qDebug() << "we split" << us.p1 << us.p2 << "|" << *isec << "|" << them.p1 << them.p2;
            Trace t1 = us;
            us.p2 = *isec;
            t1.p1 = *isec;
            int id1 = d->grp.insert(Object(t1));
            newuniverse << id << id1;
            break; // we are changed
          }
        } else {
          // collinear?
          Dim mrg = min(Dim::fromMM(0.2), (us.width + them.width) / 2 );
          if (us.collinear(them, mrg)) {
            if (us.onSegment(them.p1, mrg) && us.onSegment(them.p2, mrg)) {
              // they are fully contained
              if (them.width <= us.width) {
                // delete them
                qDebug() << "delete them" << us.p1 << us.p2 << "|" << them.p1 << them.p2 << "|" << id << nid;
                deleted << nid.first();
                d->grp.remove(nid.first());
              } else {
                // we split or truncate
                Trace &us(d->grp.object(NodeID(id)).asTrace());
                qDebug() << "we split or truncate" << us.p1 << us.p2 << "|" << them.p1 << them.p2;
                if (them.onP1(us.p1)) {
                  us.p1 = them.p2;
                } else if (them.onP2(us.p1)) {
                  us.p1 = them.p1;
                } else if (them.onP1(us.p2)) {
                  us.p2 = them.p2;
                } else if (them.onP2(us.p2)) {
                  us.p2 = them.p1;
                } else if (them.p1.distance(us.p1) < them.p2.distance(us.p1)) {
                  // our p1 closer to their p1 (so our p2 closer to their p2, because they are fully contained)
                  Trace t = us;
                  us.p2 = them.p1;
                  t.p1 = them.p2;
                  qDebug() << "   add" << t;
                  newuniverse << d->grp.insert(Object(t));
                } else {
                  Trace t = us;
                  us.p2 = them.p2;
                  t.p1 = them.p1;
                  qDebug() << "   add" << t;
                  newuniverse << d->grp.insert(Object(t));
                }
                newuniverse << id;
                qDebug() << "    we became" << us;
                break; // we are changed
              }
            } else {
              if (them.width > us.width)
                continue; // do this in their iteration instead
              if (us.p1 == them.p1 || us.p2 == them.p1
                  || us.p1 == them.p2 || us.p2 == them.p2)
                continue; // ok neighbors
              Trace &them(d->grp.object(nid).asTrace());
              qDebug() << "they truncate" << us.p1 << us.p2 << "|" << them.p1 << them.p2;
              if (us.onSegment(them.p1)) {
                // their p2 extends either beyond our p1 or our p2. move their p1 to the side of extension
                if (them.p2.distance(us.p1) < them.p2.distance(us.p2))
                  them.p1 = us.p1;
                else
                  them.p1 = us.p2;
              } else if (us.onSegment(them.p2)) {
                // their p1 extends either beyond our p1 or our p2
                if (them.p1.distance(us.p1) < them.p1.distance(us.p2))
                  them.p2 = us.p1;
                else
                  them.p2 = us.p2;
              }
              qDebug() << "    they became" << them;
              newuniverse << nid.first();
            }
          }
        }
      }
    }
    qDebug() << "new uni" << newuniverse.size() << "delete" << deleted.size() << tmr.lap();
    if (!newuniverse.isEmpty() || !deleted.isEmpty())
      any = true;
    universe = newuniverse.values();
    iter ++;
    if (iter >= 10) {
      qDebug() << "STOP";
      break;
    }
  }
  return any;
}
/*
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
*/

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
    
