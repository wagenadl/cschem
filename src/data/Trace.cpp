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

Point Trace::intersectionWith(class Trace const &t, bool *ok) const {
  if (layer == t.layer) {
    /* This implementation is really primitive; it ignores trace width */
    return Segment::intersectionWith(t, ok);
  } else {
    if (ok)
      *ok = false;
    return Point();
  }
}
