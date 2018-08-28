// Pad.cpp

#include "Pad.h"
#include "Trace.h"

Pad::Pad() {
  fpcon = false;
  noclear = false;
}

void Pad::rotate() {
  Dim x = width;
  width = height;
  height = x;
}

QXmlStreamWriter &operator<<(QXmlStreamWriter &s, Pad const &t) {
  s.writeStartElement("pad");
  s.writeAttribute("p", t.p.toString());
  s.writeAttribute("w", t.width.toString());
  s.writeAttribute("h", t.height.toString());
  s.writeAttribute("l", QString::number(int(t.layer)));
  s.writeAttribute("ref", t.ref);
  if (t.elliptic)
    s.writeAttribute("ell", "1");
  if (t.fpcon)
    s.writeAttribute("fp", "1");
  if (t.noclear)
    s.writeAttribute("noclear", "1");
  s.writeEndElement();
  return s;
}
  
QXmlStreamReader &operator>>(QXmlStreamReader &s, Pad &t) {
  bool ok;
  auto a = s.attributes();
  t.p = Point::fromString(a.value("p").toString(), &ok);
  if (ok)
    t.width = Dim::fromString(a.value("w").toString(), &ok);
  else
    t.width = Dim();
  if (ok)
    t.height = Dim::fromString(a.value("h").toString(), &ok);
  else
    t.height = Dim();
  if (ok)
    t.layer = Layer(a.value("l").toInt());
  else
    t.layer = Layer::Invalid;
  t.fpcon = a.value("fp") != 0;
  t.noclear = a.value("noclear").toInt() != 0;
  t.elliptic = a.value("ell").toInt() != 0;
  t.ref = a.value("ref").toString();
  s.skipCurrentElement();
  return s;
}

QDebug operator<<(QDebug d, Pad const &t) {
  d << "Pad(" << t.ref << t.p
    << t.width
    << t.height
    << t.layer
    << (t.elliptic ? "ell" : "")
    << (t.fpcon ? "fp" : "")
    << (t.noclear ? "noclear" : "")
    << ")";
  return d;
}

Rect Pad::boundingRect() const {
  return Rect(p - Point(width/2, height/2), p + Point(width/2, height/2));
}

Point Pad::intersectionWith(class Trace const &t, bool *ok) const {
  if (ok)
    *ok = false;
  if (t.layer != layer)
    return Point();
  if (p==t.p1 || p==t.p2)
    return Point();
  Dim diam = Dim::quadrature(width/2, height/2);
  if (t.onSegment(p, diam)) {
    // This is a slightly liberal interpretation of touching, actually, so
    // let's be a bit more careful. Following is not exact either,
    // but should be good enough in all but rather pathological cases.
    Dim min_r = width < height ? width/2 : height/2;
    if (t.onSegment(p, min_r)
	|| t.onSegment(p + Point(width/2, height/2))
	|| t.onSegment(p + Point(-width/2, height/2))
	|| t.onSegment(p + Point(width/2, -height/2))
	|| t.onSegment(p + Point(-width/2, -height/2))
	|| t.onSegment(p + Point(width/2, Dim()))
	|| t.onSegment(p + Point(-width/2, Dim()))
	|| t.onSegment(p + Point(Dim(), height/2))
	|| t.onSegment(p + Point(Dim(), -height/2))) {
      if (ok)
	*ok = true;
      return p;
    }
  }
  return Point();
}

