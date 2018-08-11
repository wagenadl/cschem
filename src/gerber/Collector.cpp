// Collector.cpp

#include "Collector.h"
#include "data/Board.h"
#include <QList>
#include <QMap>

class ColData {
public:
  Dim mirrory;
  QMap<Dim, QSet<Point>> holes;
  QMap<Dim, QSet<Point>> roundHolePads;
  QMap<Dim, QSet<Point>> squareHolePads;
  QMap<Layer, QMap<Point, QSet<Point>>> smdPads;
  QMap<Layer, QMap<Dim, QList<Trace>>> traces;
  QMap<Layer, QMap<Dim, QList<Arc>>> arcs;
  QMap<Layer, QMap<Gerber::FontSpec, QList<Text>>> texts;
};
  
Collector::Collector(Board const &brd): d(new ColData()) {
  d->mirrory = brd.height/2;
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
      d->holes[hole.id] << hole.p.flippedUpDown(d->mirrory);
      if (hole.square)
	d->squareHolePads[hole.od] << hole.p.flippedUpDown(d->mirrory);
      else
	d->roundHolePads[hole.od] << hole.p.flippedUpDown(d->mirrory);
    } break;
    case Object::Type::Pad: {
      Pad const &pad(obj.asPad());
      d->smdPads[pad.layer][Point(pad.width, pad.height)]
	<< pad.p.flippedUpDown(d->mirrory);
    } break;
    case Object::Type::Trace: {
      Trace trace(obj.asTrace());
      trace.p1 = trace.p1.flippedUpDown(d->mirrory);
      trace.p2 = trace.p2.flippedUpDown(d->mirrory);
      d->traces[trace.layer][trace.width] << trace;
    } break;
    case Object::Type::Text: {
      Text text(obj.asText());
      text.flipUpDown(d->mirrory);
      Gerber::FontSpec fs(text.fontsize, text.orient.rot, text.orient.flip);
      d->texts[text.layer][fs] << text;
    } break;
    case Object::Type::Arc: {
      Arc arc(obj.asArc());
      arc.flipUpDown(d->mirrory);
      d->arcs[arc.layer][arc.linewidth] << arc;
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

QMap<Gerber::FontSpec, QList<Text>> const &Collector::texts(Layer l) const {
  return d->texts[l];
}

