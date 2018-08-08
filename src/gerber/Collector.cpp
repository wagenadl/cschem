// Collector.cpp

#include "Collector.h"

class ColData {
public:
  QMap<Dim, QSet<Point>> holes;
  QMap<Dim, QSet<Point>> roundHolePads;
  QMap<Dim, QSet<Point>> squareHolePads;
  QMap<Layer, QMap<Point, QSet<Point>>> smdPads;
  QMap<Layer, QMap<Dim, QList<Trace>> traces;
  QMap<Layer, QMap<Dim, QList<Arc>> arcs;
  QMap<Layer, QMap<Dim, QList<Text>> texts;
};
  
Collector::Collector(): d(new ColData) {
}

Collector::~Collector() {
  delete d;
}

void Collector::collect(Group const &grp) {
  for (int id: grp.keys()) {
    Object const &obj(grp.object(id));
    switch (obj.type()) {
    case Object::Type::Group:
      collect(obj.asGroup());
      break;
    case Object::Type::Hole: {
      Hole const &hole(obj.asHole());
      holes[hole.id] << hole.p;
      if (hole.square)
	squareHolePads[hole.od] << hole.p;
      else
	roundHolePads[hole.od] << hole.p;
    } break;
    case Object::Type::Pad: {
      Pad const &pad(obj.asPad());
      smdPads[pad.layer][Point(pad.w, pad.h)] << pad.p;
    } break;
    case Object::Type::Trace: {
      Trace const &trace(obj.asTrace());
      traces[trace.layer][trace.linewidth] << trace;
    } break;
    case Object::Type::Text: {
      Text const &text(obj.asText());
      texts[text.layer][text.fontsize] << text;
    } break;
    case Object::Type::Arc: {
      Arc const &arc(obj.asArc());
      arcs[arc.layer][arc.linewidth] << arc;
    } break;
    default:
      qDebug() << "unknown type in collect";
      break;
    }
  }
}

QMap<Dim, QSet<Point>> const &Collector::holes() const {
  return d->holes;
}

QMap<Dim, QSet<Point>> const &Collector::roundHolePads() const {
  return d->roundHolePads;
}

QMap<Dim, QSet<Point>> const &Collector::squareHolePads() const {
  return d->squareHolePads;
}

QMap<Point, QSet<Point>> const &Collector::smdPads(Layer l) const {
  return d->smdPads[l];
  // not strictly const: could create empty map, but that's OK
}

QMap<Dim, QList<Trace>> const &Collector::traces(Layer l) const {
  return d->traces[l];
}

QMap<Dim, QList<Arc>> const &Collector::arcs(Layer l) const {
  return d->arcs[l];
}

QMap<Dim, QList<Text>> const &Collector::texts(Layer l) const {
  return d->texts[l];
}

