// Hole.cpp

#include "Hole.h"
#include "Trace.h"

Hole::Hole() {
  fpcon = Layer::Invalid;
  noclear = false;
}

Rect Hole::boundingRect() const {
  Dim r = od/2;
  Rect rct(p - Point(r, r), p + Point(r, r));
  return rct;
}

QXmlStreamWriter &operator<<(QXmlStreamWriter &s, Hole const &t) {
  s.writeStartElement("hole");
  s.writeAttribute("p", t.p.toString());
  s.writeAttribute("id", t.id.toString());
  s.writeAttribute("od", t.od.toString());
  s.writeAttribute("sq", t.square ? "1" : "0");
  s.writeAttribute("ref", t.ref);
  if (t.fpcon != Layer::Invalid)
    s.writeAttribute("fp", QString::number(int(t.fpcon)));
  if (t.noclear)
    s.writeAttribute("noclear", "1");
  s.writeEndElement();
  return s;
}
  
QXmlStreamReader &operator>>(QXmlStreamReader &s, Hole &t) {
  t = Hole();
  bool ok;
  auto a = s.attributes();
  t.ref = a.value("ref").toString();
  t.p = Point::fromString(a.value("p").toString(), &ok);
  if (ok)
    t.square = a.value("sq").toInt(&ok);
  if (ok)
    t.id = Dim::fromString(a.value("id").toString(), &ok);
  if (ok)
    t.od = Dim::fromString(a.value("od").toString(), &ok);
  t.noclear = a.value("noclear").toInt() != 0;
  s.skipCurrentElement();
  return s;
}

QDebug operator<<(QDebug d, Hole const &t) {
  d << "Hole("
    << t.ref
    << t.p
    << t.id
    << t.od
    << (t.square ? "square" : "circ")
    << int(t.fpcon)
    << (t.noclear ? "noclear" : "")
    << ")";
  return d;
}

bool Hole::touches(class Trace const &t) const {
  if (!(t.layer==Layer::Top || t.layer==Layer::Bottom))
    return false;
  if (p==t.p1 || p==t.p2)
    return true;
  return t.onSegment(p, od/2)
    || (square
	&& (t.onSegment(p + Point(od/2, od/2))
	    || t.onSegment(p + Point(od/2, -od/2))
	    || t.onSegment(p + Point(-od/2, od/2))
	    || t.onSegment(p + Point(-od/2, -od/2))));
}
