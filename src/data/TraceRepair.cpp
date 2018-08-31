// TraceRepair.cpp

#include "TraceRepair.h"
#include "ui/EData.h"

class RepairData {
public:
  RepairData(EData *ed): ed(ed) { }
public:
  EData *ed;
};

TraceRepair::TraceRepair(EData *ed): d(new RepairData(ed)) {
}

TraceRepair::~TraceRepair() {
  delete d;
}

void TraceRepair::fixTraceIntersections(NodeID /*id*/, Dim /*grid*/) {
  qDebug() << "fixTraceIntersections NYI";
}
 
void TraceRepair::fixPinTouchings(NodeID id) {
  QList<int> crumbs;
  int trid = id.takeLast();
  for (int k: id)
    crumbs << k;
  Group &here(d->ed->layout.root().subgroup(crumbs));
  Object &obj(here.object(trid));
  if (!obj.isTrace()) {
    qDebug() << "fixPinTouchings: not a trace";
    return;
  }

  Trace &trace(obj.asTrace());
  Intersection isec(here, trace);
  Intersection::Result res = isec.touchingPin();
  if (res.node().isEmpty())
    return;

  // So we are touching a pin. Let's split our trace up.
}

void Group::insertSegmentedTrace(Trace const &t, Dim maxsnap) {
  /* Find out whether there are any traces, holes, or pads crossed by the
     newly proposed trace. If so, break the trace into two parts at the
     crossing point (possibly mildly distorting it), and insert both parts
     recursively. Otherwise, simply insert the trace. */
  // t is specified in terms of parents coords.
  qDebug() << "insertsegmentedtrace" << t << maxsnap << t.p1.distance(t.p2).toMils();
  if (t.p1==t.p2)
    return;
  
  int id;
  Point p = intersectionWith(t, &id);
  Dim len = Point::distance(t.p1, t.p2);
  qDebug() << " => " << id << p << len;
  if (id>0 && t.p1.distance(p) < len && t.p2.distance(p) < len) {
    Object const &ocross(object(id));
    if (ocross.isTrace()) {
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
      }
    }

    if (p==t.p1 || p==t.p2) {
      // easy
      insert(Object(t));
    } else {
      Trace t1(t);
      t1.p2 = p;
      Trace t2(t);
      t2.p1 = p;
      insertSegmentedTrace(t1, maxsnap);
      insertSegmentedTrace(t2, maxsnap);
    }
  } else {
    qDebug() << "insert";
    insert(Object(t));
  }
}
