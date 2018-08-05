// Arc.cpp

#include "Arc.h"

Arc::Arc() {
  layer = Layer::Silk;
  angle = 360;
  rot = 0;
}

QXmlStreamWriter &operator<<(QXmlStreamWriter &s, Arc const &t) {
  s.writeStartElement("arc");
  s.writeAttribute("p", t.center.toString());
  s.writeAttribute("r", t.radius.toString());
  s.writeAttribute("lw", t.linewidth.toString());
  s.writeAttribute("l", QString::number(int(t.layer)));
  s.writeAttribute("ang", QString::number(t.angle));
  s.writeAttribute("rot", QString::number(t.rot));
  s.writeEndElement();
  return s;
}

 
QXmlStreamReader &operator>>(QXmlStreamReader &s, Arc &t) {
  bool ok;
  auto a = s.attributes();
  t = Arc();
  t.center = Point::fromString(a.value("p").toString(), &ok);
  if (ok)
    t.radius  = Dim::fromString(a.value("r").toString(), &ok);
  if (ok)
    t.linewidth = Dim::fromString(a.value("lw").toString(), &ok);
  if (ok) {
    if (a.contains("ext")) {
      // old style
      switch (Arc::Extent(a.value("ext").toInt(&ok))) {
      case Arc::Extent::Full:
        t.angle = 360;
        t.rot = 0;
        break;
      case Arc::Extent::LeftHalf:
        t.angle = 180;
        t.rot = 3;
        break;
      case Arc::Extent::RightHalf:
        t.angle = 180;
        t.rot = 1;
        break;
      case Arc::Extent::TopHalf:
        t.angle = 180;
        t.rot = 0;
        break;
      case Arc::Extent::BottomHalf:
        t.angle = 180;
        t.rot = 2;
        break;
      case Arc::Extent::TLQuadrant:
        t.angle = -90;
        t.rot = 3;
        break;
      case Arc::Extent::TRQuadrant:
        t.angle = -90;
        t.rot = 0;
        break;
      case Arc::Extent::BRQuadrant:
        t.angle = -90;
        t.rot = 1;
        break;
      case Arc::Extent::BLQuadrant:
        t.angle = -90;
        t.rot = 2;
        break;
      default:
        t.angle = 360;
        t.rot = 0;
        break;
      }
    } else {
      t.angle = a.value("ang").toInt(&ok);
      if (ok)
        t.rot = a.value("rot").toInt(&ok);
    }
  }
  if (ok)
    t.layer = Layer(a.value("l").toInt(&ok));
  s.skipCurrentElement();
  return s;
}


QDebug operator<<(QDebug d, Arc const &t) {
  d << "Arc(" << t.center
    << t.radius
    << t.linewidth
    << t.layer
    << t.ang << r.rot
    << ")";
  return d;
}

Rect Arc::boundingRect() const {
  // crude implementation
  qDebug() << "Arc::boundingRect: crudely implemented";
  Rect rect(center - Point(radius, radius), center + Point(radius, radius));
  rect.grow(linewidth);
  return rect;
}

bool Arc::onEdge(Point p, Dim mrg) const {
  qDebug() << "Arc::onEdge: should check angle";
  Rect br(boundingRect());
  br.grow(mrg/2);
  if (!br.contains(p))
    return false;
  Dim rmrg = (linewidth + mrg) / 2;
  Dim dist = center.distance(p);
  bool ok = dist > radius - rmrg && dist < radius + rmrg;
  return ok;
}

void Arc::flipUpDown() {
  rotateCW();
  flipLeftRight();
  rotateCW();
}

void Arc::flipLeftRight() {
  if (angle<0) {
    xxx
  }
}

void Arc::rotateCW() {
  switch (extent) {
  case Extent::LeftHalf: extent = Extent::TopHalf; break;
  case Extent::RightHalf: extent = Extent::BottomHalf; break;
  case Extent::TopHalf: extent = Extent::RightHalf; break;
  case Extent::BottomHalf: extent = Extent::LeftHalf; break;
  case Extent::TLQuadrant: extent = Extent::TRQuadrant; break;
  case Extent::TRQuadrant: extent = Extent::BRQuadrant; break;
  case Extent::BLQuadrant: extent = Extent::TLQuadrant; break;
  case Extent::BRQuadrant: extent = Extent::BLQuadrant; break;
  default: break;
  }
}

void Arc::rotateCCW() {
  rotateCW();
  rotateCW();
  rotateCW();
}

void Arc::setLayer(Layer l) {
  if ((l==Layer::Bottom) != (layer==Layer::Bottom))
    flipLeftRight();
  layer = l;
}
