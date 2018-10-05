// NPHole.cpp

#include "NPHole.h"

NPHole::NPHole() {
}

Rect NPHole::boundingRect() const {
  Dim r = d/2;
  Rect rct(p - Point(r, r), p + Point(r, r));
  return rct;
}

QXmlStreamWriter &operator<<(QXmlStreamWriter &s, NPHole const &t) {
  s.writeStartElement("nphole");
  s.writeAttribute("p", t.p.toString());
  s.writeAttribute("d", t.d.toString());
  if (t.slotlength.isPositive())
    s.writeAttribute("sl", t.slotlength.toString());
  s.writeEndElement();
  return s;
}
  
QXmlStreamReader &operator>>(QXmlStreamReader &s, NPHole &t) {
  t = NPHole();
  bool ok;
  auto a = s.attributes();
  t.p = Point::fromString(a.value("p").toString(), &ok);
  if (ok)
    t.d = Dim::fromString(a.value("d").toString(), &ok);
  if (ok)
    t.slotlength = Dim::fromString(a.value("slotlength").toString());
  s.skipCurrentElement();
  return s;
}

QDebug operator<<(QDebug d, NPHole const &t) {
  d << "NPHole("
    << t.p
    << t.d
    << t.slotlength
    << ")";
  return d;
}

