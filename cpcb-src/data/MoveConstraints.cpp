// MoveConstraints.cpp

#include "MoveConstraints.h"

enum class MoveMode {
  Stationary,
  Moving, // i.e., part of SELPTS and not STUCKPTS
  Flexible // another PUREPT; we have some wiggle room here
};

struct OtherPoint {
  Layer layer;
  Point point;
  MoveMode mode;
  OtherPoint(Layer l=Layer::Invalid, Point p=Point(),
	     MoveMode m=MoveMode::Stationary):
    layer(l), point(p), mode(m) { }
};

class MCData {
public:
  MCData() {
  }
public:
  QMap<Layer, QMap<Point, QList<OtherPoint>> cons;
  Point delta;
};

MoveConstraints::MoveConstraints(): d(new  MCData()) {
}

MoveConstraints::~MoveConstraints() {
  delete d;
}

MoveConstraints::MoveConstraints(Group const &root,
				 QSet<int> const &selection,
				 QMap<Layer, QSet<Point>> const &selpts,
				 QMap<Layer, QSet<Point>> const &purepts,
				 QMap<Layer, QSet<Point>> const &stuckpts):
  MoveConstraints() {
  // find all traces
  QMap<Layer, QMap<Point, QSet<Point>>> trcs;
  for (int id: root.keys()) {
    Object const &obj(root.object(id));
    if (obj.isTrace()) {
      Trace const &t(obj.asTrace());
      trcs[t.layer][t.p1].insert(t.p2);
      trcs[t.layer][t.p2].insert(t.p1);
    }
  }
  // make our connections database...
  for (Layer l: purepts.keys()) {
    for (Point p: purepts[l]) {
      for (Point p1: trcs[l][p]) {
	MoveMode m = purepts.contains(p1)
	  ? MoveMode::Flexible
	  : (selpts.contains(p1) && !stuckpts.contains(p1))
	  ? MoveMode::Moving
	  : MoveMode::Stationary;
	d->cons[l][p] << OtherPoint(l, p1, m);
      }
    }
  }	
}

void MoveConstraints::setMovingDelta(Point const &delta) {
  d->delta = delta;
}

Point MoveConstraints::apply(Layer layer, Point const &origpt) {
  // for now:
  return origpt + d->delta;
}
