// Hole.cpp

#include "Hole.h"

Hole::Hole() {
}

Rect Hole::boundingRect() const {
  Dim r = od/2;
  return Rect(p - Point(r, r), p + Point(r, r));
}

QXmlStreamWriter &operator<<(QXmlStreamWriter &s, Hole const &t) {
  s.writeStartElement("hole");
  s.writeAttribute("p", t.p.toString());
  s.writeAttribute("id", t.id.toString());
  s.writeAttribute("od", t.od.toString());
  s.writeAttribute("sq", t.square ? "1" : "0");
  s.writeAttribute("ref", t.ref);
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
    << ")";
  return d;
}
