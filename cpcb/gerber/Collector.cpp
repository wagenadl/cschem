// Collector.cpp

#include "Collector.h"
#include "data/Board.h"
#include <QList>
#include <QMap>

class ColData {
public:
  Dim mirrory;
  QMap<Dim, QList<Hole>> holes;
  QMap<Dim, QList<NPHole>> npHoles;
  QMap<Layer, QMap<Dim, QList<Hole>>> roundHolePads;
  QMap<Layer, QMap<Dim, QList<Hole>>> squareHolePads;
  QMap<Layer, QMap<Point, QList<Pad>>> smdPads;
  QMap<Layer, QMap<Dim, QList<Trace>>> traces;
  QMap<Layer, QMap<Dim, QList<Arc>>> arcs;
  QMap<Layer, QList<Polyline>> filledPlanes;
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
      Hole hole(obj.asHole());
      hole.flipUpDown(d->mirrory);
      d->holes[hole.id] << hole;
      auto &map(hole.square ? d->squareHolePads : d->roundHolePads);
      map[Layer::Top][hole.od] << hole;
      map[Layer::Bottom][hole.od] << hole;
    } break;
    case Object::Type::NPHole: {
      NPHole hole(obj.asNPHole());
      hole.flipUpDown(d->mirrory);
      d->npHoles[hole.d] << hole;
    } break;
    case Object::Type::Pad: {
      Pad pad(obj.asPad());
      pad.flipUpDown(d->mirrory);
      d->smdPads[pad.layer][Point(pad.width, pad.height)] << pad;
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
      if (text.layer==Layer::Bottom)
        text.flip = !text.flip;
      Gerber::FontSpec fs(text.fontsize, text.rota, text.flip);
      d->texts[text.layer][fs] << text;
    } break;
    case Object::Type::Arc: {
      Arc arc(obj.asArc());
      arc.flipUpDown(d->mirrory);
      d->arcs[arc.layer][arc.linewidth] << arc;
    } break;
    case Object::Type::Plane: {
      FilledPlane fp(obj.asPlane());
      fp.flipUpDown(d->mirrory);
      d->filledPlanes[fp.layer] << fp.perimeter;
    } break;
    default:
      qDebug() << "unknown type in collect";
      break;
    }
  }
}

QMap<Dim, QList<Hole>> const &Collector::holes() const {
  return d->holes;
}

QMap<Dim, QList<NPHole>> const &Collector::npHoles() const {
  return d->npHoles;
}

QMap<Dim, QList<Hole>> const &
  Collector::roundHolePads(Layer l) const {
  return d->roundHolePads[l];
}

QMap<Dim, QList<Hole>> const &
  Collector::squareHolePads(Layer l) const {
  return d->squareHolePads[l];
}

QMap<Point, QList<Pad>> const &Collector::smdPads(Layer l) const {
  return d->smdPads[l];
}

QMap<Dim, QList<Trace>> const &Collector::traces(Layer l) const {
  return d->traces[l];
}

QMap<Dim, QList<Arc>> const &Collector::arcs(Layer l) const {
  return d->arcs[l];
}

QList<Polyline> const &Collector::filledPlanes(Layer l) const {
  return d->filledPlanes[l];
}

QMap<Gerber::FontSpec, QList<Text>> const &Collector::texts(Layer l) const {
  return d->texts[l];
}

