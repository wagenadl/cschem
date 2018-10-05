// Arc.cpp

#include "Arc.h"

Arc::Arc() {
  layer = Layer::Silk;
  angle = 360;
}

QXmlStreamWriter &operator<<(QXmlStreamWriter &s, Arc const &t) {
  s.writeStartElement("arc");
  s.writeAttribute("p", t.center.toString());
  s.writeAttribute("r", t.radius.toString());
  s.writeAttribute("lw", t.linewidth.toString());
  s.writeAttribute("l", QString::number(int(t.layer)));
  s.writeAttribute("ang", QString::number(t.angle));
  s.writeAttribute("rota", QString::number(t.rota));
  s.writeEndElement();
  return s;
}

 
QXmlStreamReader &operator>>(QXmlStreamReader &s, Arc &t) {
  bool ok;
  auto a = s.attributes();
  t = Arc();
  t.center = Point::fromString(a.value("p").toString(), &ok);
  t.radius  = Dim::fromString(a.value("r").toString(), &ok);
  t.linewidth = Dim::fromString(a.value("lw").toString(), &ok);
  if (a.hasAttribute("ext")) {
    t.setExtent(Arc::Extent(a.value("ext").toInt(&ok)));
  } else {
    t.angle = a.value("ang").toInt(&ok);
    if (a.hasAttribute("rot")) {
      /* In older versions, if rot=0, arc was centred around 12 o'clock,
         unless angle was negative,
         in which case arc ran clockwise from 12 o'clock. */
      t.rota = FreeRotation(a.value("rot").toInt(&ok) * 90);
      if (t.angle<0) 
        t.angle = -t.angle;
      else
        t.rota -= t.angle/2;
    } else {
      t.rota = FreeRotation(a.value("rota").toInt(&ok));
    }
  }
  t.layer = Layer(a.value("l").toInt(&ok));
  s.skipCurrentElement();
  return s;
}


QDebug operator<<(QDebug d, Arc const &t) {
  d << "Arc(" << t.center
    << t.radius
    << t.linewidth
    << t.layer
    << t.angle << t.rota
    << ")";
  return d;
}

Rect Arc::boundingRect() const {
  Rect rect(center, center);
  int a0 = rota - 90;
  int a1 = a0 + angle;
  constexpr double PI = 4*atan(1);
  for (int a=a0; a<=a1; a+=15) {
    double phi = a*PI/180;
    rect |= center + Point(radius*cos(phi), radius*sin(phi));
  }
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

void Arc::flipUpDown(Dim y) {
  flipUpDown();
  center = center.flippedUpDown(y);
}

void Arc::flipLeftRight() {
  rota += angle;
  rota = -rota;
}

void Arc::rotateCW() {
  rota += 90;
}

void Arc::rotateCCW() {
  rota -= 90;
}

void Arc::setLayer(Layer l) {
  if ((l==Layer::Bottom) != (layer==Layer::Bottom))
    flipLeftRight();
  layer = l;
}

Arc::Extent Arc::extent() const {
  return Extent::Invalid; // not actually supported...
}

void Arc::setExtent(Arc::Extent e) {
  // ancient style
  switch (e) {
  case Arc::Extent::Full:
    angle = 360;
    rota = FreeRotation(0);
    break;
  case Arc::Extent::LeftHalf:
    angle = 180;
    rota = FreeRotation(180);
    break;
  case Arc::Extent::RightHalf:
    angle = 180;
    rota = FreeRotation(0);
    break;
  case Arc::Extent::TopHalf:
    angle = 180;
    rota = FreeRotation(270);
    break;
  case Arc::Extent::BottomHalf:
    angle = 180;
    rota = FreeRotation(90);
    break;
  case Arc::Extent::TLQuadrant:
    angle = 90;
    rota = FreeRotation(270);
    break;
  case Arc::Extent::TRQuadrant:
    angle = 90;
    rota = FreeRotation(0);
    break;
  case Arc::Extent::BRQuadrant:
    angle = 90;
    rota = FreeRotation(90);
    break;
  case Arc::Extent::BLQuadrant:
    angle = 90;
    rota = FreeRotation(180);
    break;
  default:
    angle = 360;
    rota = FreeRotation(0);
    break;
  }
}
