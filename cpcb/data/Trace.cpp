// Trace.cpp

#include "Trace.h"

Trace::Trace() {
  layer = Layer::Invalid;
}

QXmlStreamWriter &operator<<(QXmlStreamWriter &s, Trace const &t) {
  s.writeStartElement("trace");
  s.writeAttribute("p1", t.p1.toString());
  s.writeAttribute("p2", t.p2.toString());
  s.writeAttribute("w", t.width.toString());
  s.writeAttribute("l", QString::number(int(t.layer)));
  s.writeEndElement();
  return s;
}
  
QXmlStreamReader &operator>>(QXmlStreamReader &s, Trace &t) {
  bool ok;
  t = Trace();
  auto a = s.attributes();
  t.p1 = Point::fromString(a.value("p1").toString(), &ok);
  if (ok)
    t.p2 = Point::fromString(a.value("p2").toString(), &ok);
  if (ok)
    t.width = Dim::fromString(a.value("w").toString(), &ok);
  if (ok)
    t.layer = Layer(a.value("l").toInt());
  s.skipCurrentElement();
  return s;
}

QDebug operator<<(QDebug d, Trace const &t) {
  d << "Trace(" << t.p1
    << t.p2
    << t.width
    << t.layer
    << ")";
  return d;
}

Rect Trace::boundingRect() const {
  return Segment::boundingRect().grow(width);
}

bool Trace::onP1(Point p, Dim mrg) const {
  return Segment::onP1(p, mrg + width/2);
}

bool Trace::onP2(Point p, Dim mrg) const {
  return Segment::onP2(p, mrg + width/2);
}

bool Trace::onSegment(Point p, Dim mrg) const {
  return Segment::onSegment(p, mrg + width/2);
}

bool Trace::touches(class Trace const &t, Point *pt) const {
  // this is ridiculously elaborate, but more or less correct
  if (layer != t.layer)
    return false;
  if (!boundingRect().intersects(t.boundingRect()))
    return false;

  if (t.onSegment(p1, width/2)) {
    if (pt)
      *pt = p1;
    return true;
  } else if (t.onSegment(p2, width/2)) {
    if (pt)
      *pt = p1;
    return true;
  } else if (onSegment(t.p1, t.width/2)) {
    if (pt)
      *pt = t.p1;
    return true;
  } else if (onSegment(t.p2, t.width/2)) {
    if (pt)
      *pt = t.p2;
    return true;
  }
  
  if (intersects(t, pt))
    return true;
  else if (orthogonallyDisplaced(width/2).intersects(t, pt)
      || orthogonallyDisplaced(-width/2).intersects(t, pt))
    return true;
  else if (intersects(t.orthogonallyDisplaced(t.width/2), pt)
      || intersects(t.orthogonallyDisplaced(-t.width/2), pt))
    return true;
  else
    return false;
}

bool Trace::touches(class Segment const &t, Point *pt) const {
  if (!boundingRect().intersects(t.boundingRect()))
    return false;

  if (t.onSegment(p1, width/2)) {
    if (pt)
      *pt = p1;
    return true;
  } else if (t.onSegment(p2, width/2)) {
    if (pt)
      *pt = p1;
    return true;
  } else if (onSegment(t.p1)) {
    if (pt)
      *pt = t.p1;
    return true;
  } else if (onSegment(t.p2)) {
    if (pt)
      *pt = t.p2;
    return true;
  }

  if (intersects(t, pt))
    return true;
  else if (orthogonallyDisplaced(width/2).intersects(t, pt)
      || orthogonallyDisplaced(-width/2).intersects(t, pt))
    return true;
  else
    return false;
}

bool Trace::operator==(Trace const &t) const {
  return layer==t.layer && width==t.width && p1==t.p1 && p2==t.p2;
}

bool Trace::touches(Rect r) const {
  r.grow(width);
  return intersects(r);
}
