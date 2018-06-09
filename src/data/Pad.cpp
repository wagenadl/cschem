// Pad.cpp

#include "Pad.h"

Pad::Pad() {
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
  if (t.elliptic)
    s.writeAttribute("ell", "1");
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
    t.layer = Layer(a.value("layer").toInt());
  else
    t.layer = Layer::Invalid;
  t.elliptic = a.value("ell").toInt() != 0;
  s.skipCurrentElement();
  return s;
}

QDebug operator<<(QDebug d, Pad const &t) {
  d << "Pad(" << t.p
    << t.width
    << t.height
    << t.layer
    << (t.elliptic ? "ell" : "")
    << ")";
  return d;
}

Rect Pad::boundingRect() const {
  return Rect(p - Point(width/2, height/2), p + Point(width/2, height/2));
}
