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
  while (!universe.isEmpty()) {
    QSet<int> newuniverse;
    NetGraph ng(d->grp);
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
          Trace &us(d->grp.object(NodeID(id)).asTrace());
          _result.maxmove = max(_result.maxmove, us.p1.distance(p));
          us.p1 = p;
          newuniverse << id;
          _result.nmoved_pin ++;
          break;
        } else if (us.onP2(p)) {
          Trace &us(d->grp.object(NodeID(id)).asTrace());
          _result.maxmove = max(_result.maxmove, us.p2.distance(p));
          us.p2 = p;
          newuniverse << id;
          _result.nmoved_pin ++;
          break;
        } else if (us.projectsWithin(p)) {
          // pin touches us, but not near endpoint
          Trace &us(d->grp.object(NodeID(id)).asTrace());
          Trace t1 = us;
          us.p2 = p;
          t1.p1 = p;
          int id1 = d->grp.insert(Object(t1));
          newuniverse << id << id1;
          _result.nsplit_pin ++;
          break;
        }
      }
    }
    universe = newuniverse.values();
    if (universe.size())
      any = true;
  }
  return any;
}

bool TraceRepair::fixTraceIntersections() {
  bool any = false;
  QList<int> universe = d->grp.keys();
  int iter = 0;
  while (!universe.isEmpty()) {
    QSet<int> newuniverse;
    QSet<int> deleted;
    NetGraph ng(d->grp);
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
            Trace t1 = them;
            them.p2 = *isec;
            t1.p1 = *isec;
            int id1 = d->grp.insert(Object(t1));
            newuniverse << id << id1;
            _result.nsplit_intersect ++;
          }
          if (us.betweenEndpoints(*isec, us.width/2)) {
            // we split
            Trace &us(d->grp.object(NodeID(id)).asTrace());
            Trace t1 = us;
            us.p2 = *isec;
            t1.p1 = *isec;
            int id1 = d->grp.insert(Object(t1));
            newuniverse << id << id1;
            _result.nsplit_intersect ++;
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
                deleted << nid.first();
                d->grp.remove(nid.first());
                _result.ndeleted_overlap ++;
              } else {
                // we split or truncate
                Trace &us(d->grp.object(NodeID(id)).asTrace());
                if (them.onP1(us.p1)) {
                  qDebug() << "moving us p1 to them.p2" << us << them;
                  us.p1 = them.p2;
                  _result.nmoved_overlap ++;
                } else if (them.onP2(us.p1)) {
                  qDebug() << "moving us p1 to them.p1" << us << them;
                  us.p1 = them.p1;
                  _result.nmoved_overlap ++;
                } else if (them.onP1(us.p2)) {
                  qDebug() << "moving us p2 to them.p2" << us << them;
                  us.p2 = them.p2;
                  _result.nmoved_overlap ++;
                } else if (them.onP2(us.p2)) {
                  qDebug() << "moving us p2 to them.p1" << us << them;
                  us.p2 = them.p1;
                  _result.nmoved_overlap ++;
                } else if (them.p1.distance(us.p1) < them.p2.distance(us.p1)) {
                  // our p1 closer to their p1 (so our p2 closer to their p2, because they are fully contained)
                  Trace t = us;
                  qDebug() << "split us at us.p1/them.p1" << us << them;
                  us.p2 = them.p1;
                  t.p1 = them.p2;
                  newuniverse << d->grp.insert(Object(t));
                  _result.nmoved_overlap ++;
                } else {
                  Trace t = us;
                  qDebug() << "split us at us.p1/them.p2" << us << them;
                  us.p2 = them.p2;
                  t.p1 = them.p1;
                  newuniverse << d->grp.insert(Object(t));
                  _result.nmoved_overlap ++;
                }
                newuniverse << id;
                break; // we are changed
              }
            } else {
              // not fully contained, but overlapping collinear
              if (them.width > us.width)
                continue; // do this in their iteration instead
              if (us.p1 == them.p1 || us.p2 == them.p1
                  || us.p1 == them.p2 || us.p2 == them.p2)
                continue; // ok neighbors
              Trace &them(d->grp.object(nid).asTrace());
              if (us.onSegment(them.p1)) {
                // their p2 extends either beyond our p1 or our p2. move their p1 to the side of extension
                qDebug() << "moving them.p1" << us << them;
                if (them.p2.distance(us.p1) < them.p2.distance(us.p2))
                  them.p1 = us.p1;
                else
                  them.p1 = us.p2;
                _result.nmoved_overlap ++;
                newuniverse << nid.first();
              } else if (us.onSegment(them.p2)) {
                qDebug() << "moving them.p2" << us << them;
                // their p1 extends either beyond our p1 or our p2
                if (them.p1.distance(us.p1) < them.p1.distance(us.p2))
                  them.p2 = us.p1;
                else
                  them.p2 = us.p2;
                _result.nmoved_overlap ++;
                newuniverse << nid.first();
              } else {
                qDebug() << "not moving them" << us << them;
              }
            }
          }
        }
      }
    }
    if (!newuniverse.isEmpty() || !deleted.isEmpty())
      any = true;
    universe = newuniverse.values();
    iter ++;
    qDebug() << "iter" << iter;
      qDebug() << "newuniverse" << newuniverse;
      for (int id: newuniverse) {
        Object const &obj = d->grp.object(id);
        if (obj.isTrace()) {
          qDebug() << "  " << id << ":" << obj.asTrace();
        }
      }
      qDebug() << "deleted" << deleted;
    if (iter >= 10) {
      qDebug() << "STOP - Too many iterations in TraceRepair";
      break;
    }
  }
  return any;
}


bool TraceRepair::dropDanglingTraces() {
  bool any = false;
  QList<int> universe = d->grp.keys();
  while (!universe.isEmpty()) {
    QSet<int> newuniverse;
    NetGraph ng(d->grp);
    QSet<int> dropme;
    for (int id: universe) {
      Object const &obj(d->grp.object(id));
      if (obj.isHole() && obj.asHole().via) {
        auto adj = ng.adjacent(NodeID(id));
        if (adj.size() <= 1) {
          dropme << id;
          _result.ndeleted_dangling_via ++;
          for (NodeID const &nid: adj)
            newuniverse << nid.first();
        }
      } else if (obj.isTrace()) {
        Trace const &tr = obj.asTrace();
        auto adj = ng.adjacent(NodeID(id));
        if (adj.size() <= 1) {
          dropme << id;
          _result.ndeleted_dangling ++;
          for (NodeID const &nid: adj)
            newuniverse << nid.first();
        } else {
          int atp1 = 0;
          int atp2 = 0;
          for (NodeID const &nid: adj) {
            Object const &obj1(d->grp.object(nid));
            if (obj1.isTrace()) {
              Trace const &tr1(obj1.asTrace());
              if (tr.onP1(tr1.p1) || tr.onP1(tr1.p2))
                atp1 ++;
              else if (tr.onP2(tr1.p1) || tr.onP2(tr1.p2))
                atp2 ++;
            } else if (obj1.isHoleOrPad()) {
              Point p = obj1.point();
              if (tr.onP1(p))
                atp1 ++;
              else if (tr.onP2(p))
                atp2 ++;
            }
          }
          if (atp1 == adj.size() || atp2 == adj.size()) {
            dropme << id;
            _result.ndeleted_dangling ++;
            for (NodeID const &nid: adj)
              newuniverse << nid.first();
          }
        }
      }
    }
    for (int id: dropme) 
      d->grp.remove(id);
    if (dropme.size())
      any = true;
    universe = newuniverse.values();
  }
  return any;
}
    
bool TraceRepair::fixTraceFragments() {
  /* A trace T is a fragment that can be absorbed into another trace U if all
     of the following are true:
     - T and U are collinear
     - T and U have the same width
     - One of T's endpoints (P = T.p1 or T.p2) lies exactly on an endpoint of U
     - T does not touch any pins or pads at P
     - Other than U, T does not touch any traces at P

     We iterate over all traces T, checking whether they meet the conditions.
     If so, absorb T into U and mark U for re-evaluation in a subsequent pass.
     Once nothing is left to evaluate, we are done.
  */

  bool any = false;
  QList<int> universe = d->grp.keys();
  TicToc tmr;
  while (!universe.isEmpty()) {
    QSet<int> newuniverse;
    NetGraph ng(d->grp);
    //    qDebug() << "fragments" << tmr.lap();
    for (int id: universe) {
      if (newuniverse.contains(id))
        continue; // save for next round
      Object const &obj(d->grp.object(id));
      if (!obj.isTrace())
        continue;
      Trace const &us(obj.asTrace());
      int adjcountatp1 = 0;
      int adjcountatp2 = 0;
      NodeID col;
      int at = 0; // 1 for p1 2 for p2
      for (NodeID const &nid: ng.adjacent(NodeID(id))) {
        Object const &obj1(d->grp.object(nid));
        if (obj1.isHoleOrPad()) {
          Point p(obj1.point());
          if (us.onP1(p))
            adjcountatp1 ++;
          if (us.onP2(p))
            adjcountatp2 ++;
        } else if (obj1.isTrace()) {
          Trace const &them(obj1.asTrace());
          if (us.onP1(them.p1) || us.onP1(them.p2)) {
            adjcountatp1 ++;
            if (us.width == them.width
                && (us.p1 == them.p1 || us.p1 == them.p2)
                && us.collinear(them)) {
              col = nid;
              at = 1;
            }
          }
          if (us.onP2(them.p1) || us.onP2(them.p2)) {
            adjcountatp2 ++;
            //            qDebug() << "?" << them << us.collinear(them);
            if (us.width == them.width
                && (us.p2 == them.p1 || us.p2 == them.p2)
                && us.collinear(them)) {
              col = nid;
              at = 2;
            }
          }
        }          
      }
      //      qDebug() << "us" << us << adjcountatp1 << adjcountatp2 << at << col;
      if (at == 1 && adjcountatp1 == 1) {
        // we are absorbable at p1
        Trace &them(d->grp.object(col).asTrace());
        //        qDebug() << "  absorb" << us << "into" << them << "at p1";
        if (us.onP1(them.p1))
          them.p1 = us.p2;
        else
          them.p2 = us.p2;
        newuniverse << col.first();
        d->grp.remove(id);
        _result.njoined ++;
      } else if (at == 2 && adjcountatp2 == 1) {
        // we are absorbable at p2
        Trace &them(d->grp.object(col).asTrace());
        //        qDebug() << "  absorb" << us << "into" << them << "at p2";
        if (us.onP2(them.p1))
          them.p1 = us.p1;
        else
          them.p2 = us.p1;
        newuniverse << col.first();
        d->grp.remove(id);
        _result.njoined ++;
      }
    }
    universe = newuniverse.values();
    if (universe.size())
      any = true;
  }
  return any;
}
