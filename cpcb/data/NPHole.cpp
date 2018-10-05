// NPHole.cpp

#include "NPHole.h"

NPHole::NPHole() {
}

Rect NPHole::boundingRect() const {
  Dim r = d/2;
  if (slotlength.isPositive()) {
    constexpr double PI = 4*atan(1);
    Point dxy(cos(rota*PI/180)*slotlength/2, sin(rota*PI/180)*slotlength/2);
    return Rect(p - dxy, p + dxy).grow(d);
  } else {
    return Rect(p - Point(r, r), p + Point(r, r));
  }
}

QXmlStreamWriter &operator<<(QXmlStreamWriter &s, NPHole const &t) {
  s.writeStartElement("nphole");
  s.writeAttribute("p", t.p.toString());
  s.writeAttribute("d", t.d.toString());
  if (t.slotlength.isPositive())
    s.writeAttribute("sl", t.slotlength.toString());
  if (t.rota)
    s.writeAttribute("rot", QString::number(t.rota));
  s.writeEndElement();
  return s;
}
  
QXmlStreamReader &operator>>(QXmlStreamReader &s, NPHole &t) {
  t = NPHole();
  bool ok;
  auto a = s.attributes();
  t.p = Point::fromString(a.value("p").toString(), &ok);
  t.d = Dim::fromString(a.value("d").toString(), &ok);
  t.slotlength = Dim::fromString(a.value("slotlength").toString());
  t.rota = FreeRotation(a.value("rot").toInt());
  s.skipCurrentElement();
  return s;
}

QDebug operator<<(QDebug d, NPHole const &t) {
  d << "NPHole("
    << t.p
    << t.d
    << t.slotlength
    << t.rota
    << ")";
  return d;
}

