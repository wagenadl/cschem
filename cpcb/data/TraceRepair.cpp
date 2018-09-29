// TraceRepair.cpp

#include "TraceRepair.h"
#include "Group.h"
#include "Intersection.h"
#include "Object.h"

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
      fixTraceIntersections(res.node[0], grid);
    }
  }
}
/*
      Trace const &tcross(ocross.asTrace());
      Dim lcr = Point::distance(tcross.p1, tcross.p2);
      if (p!=tcross.p1 && p!=tcross.p2
          && p.distance(tcross.p1)<lcr && p.distance(tcross.p2)<lcr) {
        // we are crossing somewhere in the middle of the crossing trace
        // => break the crossing trace up
	Trace tc1(tcross);
	tc1.p2 = p;
	Trace tc2(tcross);
	tc2.p1 = p;
	remove(id);
	insert(Object(tc1));
	insert(Object(tc2));
*/
 
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
  QSet<Point> fixedpoints = d->grp.pinPoints();
  QMap<Point, int> tracepoints;
  for (int id: d->grp.keys()) {
    Object const &obj(d->grp.object(id));
    if (obj.isTrace()) {
      Trace const &t(obj.asTrace());
      if (t.layer==Layer::Top || t.layer==Layer::Bottom) {
	tracepoints[t.p1]++;
	tracepoints[t.p2]++;
      }
    }
  } 
  bool anyever = false;
  while (true) {
    QSet<int> dropme;
    for (int id: d->grp.keys()) {
      Object const &obj(d->grp.object(id));
      if (obj.isTrace()) {
	Trace const &t(obj.asTrace());
	if (t.layer==Layer::Top || t.layer==Layer::Bottom) {
	  if (!(tracepoints[t.p1]>=2 || fixedpoints.contains(t.p1))
	      || !(tracepoints[t.p2]>=2 || fixedpoints.contains(t.p2))
	      || t.p1 == t.p2) {
	    dropme << id;
	    tracepoints[t.p1]--;
	    tracepoints[t.p2]--;
	  }
	}
      }
    }
    if (dropme.isEmpty())
      break;
    for (int id: dropme)
      d->grp.remove(id);
    anyever = true;
  }
  return anyever;
}
    
