// Collector.cpp

#include "Collector.h"
#include "data/Board.h"
#include <QList>
#include <QMap>

class ColData {
public:
  Dim mirrory;
  QMap<Dim, QSet<Point>> holes;
  QMap<Layer, QMap<Dim, QList<Collector::PadInfo>>> roundHolePads;
  QMap<Layer, QMap<Dim, QList<Collector::PadInfo>>> squareHolePads;
  QMap<Layer, QMap<Point, QList<Collector::PadInfo>>> smdPads;
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
      Hole const &hole(obj.asHole());
      Point p(hole.p.flippedUpDown(d->mirrory));
      d->holes[hole.id] << p;
      PadInfo padi;
      padi.p = p;
      padi.noclear = hole.noclear;
      auto &map(hole.square ? d->squareHolePads : d->roundHolePads);
      padi.fpcon = hole.fpcon==Layer::Top;
      map[Layer::Top][hole.od] << padi;
      padi.fpcon = hole.fpcon==Layer::Bottom;
      map[Layer::Bottom][hole.od] << padi;
    } break;
    case Object::Type::Pad: {
      Pad const &pad(obj.asPad());
      PadInfo padi;
      padi.p = pad.p.flippedUpDown(d->mirrory);
      padi.noclear = pad.noclear;
      padi.fpcon = pad.fpcon;
      d->smdPads[pad.layer][Point(pad.width, pad.height)] << padi;
    } break;
    case Object::Type::Trace: {
      Trace trace(obj.asTrace());
      trace.p1 = trace.p1.flippedUpDown(d->mirrory);
      trace.p2 = trace.p2.flippedUpDown(d->mirrory);
      d->traces[trace.layer][trace.width] << trace;
    } break;
    case Object::Type::Text: {
      Text text(obj.asText());
      qDebug() << "collected" << text;
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

QMap<Dim, QSet<Point>> const &Collector::holes() const {
  return d->holes;
}

QMap<Dim, QList<Collector::PadInfo>> const &
  Collector::roundHolePads(Layer l) const {
  return d->roundHolePads[l];
}

QMap<Dim, QList<Collector::PadInfo>> const &
  Collector::squareHolePads(Layer l) const {
  return d->squareHolePads[l];
}

QMap<Point, QList<Collector::PadInfo>> const &Collector::smdPads(Layer l) const {
  return d->smdPads[l];
  // not strictly const: could create empty map, but that's OK
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

