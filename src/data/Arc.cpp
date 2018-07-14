// Arc.cpp

#include "Arc.h"

Arc::Arc() {
  layer = Layer::Silk;
  extent = Extent::Full;
}

QXmlStreamWriter &operator<<(QXmlStreamWriter &s, Arc const &t) {
  s.writeStartElement("arc");
  s.writeAttribute("p", t.center.toString());
  s.writeAttribute("r", t.radius.toString());
  s.writeAttribute("lw", t.linewidth.toString());
  s.writeAttribute("l", QString::number(int(t.layer)));
  s.writeAttribute("ext", QString::number(int(t.extent)));
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
  if (ok)
    t.extent = Arc::Extent(a.value("ext").toInt(&ok));
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
    << int(t.extent)
    << ")";
  return d;
}

Rect Arc::boundingRect() const {
  Point p0(center);
  Point p1(center);
  Dim zero;
  switch (extent) {
  case Extent::Invalid:
    break;
  case Extent::Full:
    p0 += Point(-radius, -radius);
    p1 += Point(radius, radius);
    break;
  case Extent::LeftHalf:
    p0 += Point(-radius, -radius);
    p1 += Point(zero, radius);
    break;
  case Extent::RightHalf:
    p0 += Point(zero, -radius);
    p1 += Point(radius, radius);
    break;
  case Extent::TopHalf:
    p0 += Point(-radius, -radius);
    p1 += Point(radius, zero);
    break;
  case Extent::BottomHalf:
    p0 += Point(-radius, zero);
    p1 += Point(radius, radius);
    break;
  case Extent::TLQuadrant:
    p0 += Point(-radius, -radius);
    break;
  case Extent::TRQuadrant:
    p0 += Point(radius, -radius);
    break;
  case Extent::BLQuadrant:
    p0 += Point(-radius, radius);
    break;
  case Extent::BRQuadrant:
    p0 += Point(radius, radius);
    break;
  }
  Rect rect(p0, p1);
  rect.grow(linewidth);
  return rect;
}

bool Arc::onEdge(Point p, Dim mrg) const {
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
  switch (extent) {
  case Extent::LeftHalf: extent = Extent::RightHalf; break;
  case Extent::RightHalf: extent = Extent::LeftHalf; break;
  case Extent::TLQuadrant: extent = Extent::TRQuadrant; break;
  case Extent::TRQuadrant: extent = Extent::TLQuadrant; break;
  case Extent::BLQuadrant: extent = Extent::BRQuadrant; break;
  case Extent::BRQuadrant: extent = Extent::BLQuadrant; break;
  default: break;
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
