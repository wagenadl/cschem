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
    if (a.hasAttribute("ext")) {
      t.setExtent(Arc::Extent(a.value("ext").toInt(&ok)));
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
    << t.angle << t.rot
    << ")";
  return d;
}

int startangle(Arc const &a) {
  // 0 is +x (right), 90 is +y (down)
  int a0 = 90 * (a.rot&3) - 90;
  if (a.angle<0) 
    return a0;
  else
    return a0 - a.angle/2;
}

int endangle(Arc const &a) {
  int a0 = 90 * (a.rot&3) - 90;
  if (a.angle<0)
    return a0 - a.angle;
  else
    return a0 + a.angle/2;
}

Rect Arc::boundingRect() const {
  Rect rect(center, center);
  int a0 = startangle(*this);
  int a1 = endangle(*this);
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

void Arc::flipLeftRight() {
  rot &= 3;
  if (angle<0) {
    rot = 3 - rot;
  } else {
    if (rot==1)
      rot = 3;
    else if (rot==3)
      rot = 1;
  }
}

void Arc::rotateCW() {
  rot = (rot + 1) & 3;
}

void Arc::rotateCCW() {
  rot = (rot - 1) & 3;
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
  // old style
  switch (e) {
  case Arc::Extent::Full:
    angle = 360;
    rot = 0;
    break;
  case Arc::Extent::LeftHalf:
    angle = 180;
    rot = 3;
    break;
  case Arc::Extent::RightHalf:
    angle = 180;
    rot = 1;
    break;
  case Arc::Extent::TopHalf:
    angle = 180;
    rot = 0;
    break;
  case Arc::Extent::BottomHalf:
    angle = 180;
    rot = 2;
    break;
  case Arc::Extent::TLQuadrant:
    angle = -90;
    rot = 3;
    break;
  case Arc::Extent::TRQuadrant:
    angle = -90;
    rot = 0;
    break;
  case Arc::Extent::BRQuadrant:
    angle = -90;
    rot = 1;
    break;
  case Arc::Extent::BLQuadrant:
    angle = -90;
    rot = 2;
    break;
  default:
    angle = 360;
    rot = 0;
    break;
  }
}
