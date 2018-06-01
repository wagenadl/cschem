// Text.cpp

#include "Text.h"

Text::Text() {
}

QXmlStreamWriter &operator<<(QXmlStreamWriter &s, Text const &t) {
  s.writeStartElement("text");
  s.writeAttribute("p", t.p.toString());
  s.writeAttribute("fs", t.fontsize.toString());
  s.writeAttribute("l", QString::number(int(t.layer)));
  s.writeAttribute("ori", t.orient.toString());
  s.writeAttribute("text", t.text);
  s.writeEndElement();
  return s;
}
  
QXmlStreamReader &operator>>(QXmlStreamReader &s, Text &t) {
  bool ok;
  auto a = s.attributes();
  t.p = Point::fromString(a.value("p").toString(), &ok);
  if (ok)
    t.fontsize = Dim::fromString(a.value("fs").toString(), &ok);
  else
    t.fontsize = Dim();
  if (ok)
    t.orient = Orient::fromString(a.value("ori").toString(), &ok);
  else
    t.orient = Orient();
  if (ok)
    t.text = a.value("text").toString();
  else
    t.text = "";
  if (ok)
    t.layer = Layer(a.value("layer").toInt());
  else
    t.layer = Layer::Invalid;
  s.skipCurrentElement();
  qDebug() << "got text" << t << ok;
  return s;
}

QDebug operator<<(QDebug d, Text const &t) {
  d << "Text(" << t.p
    << t.layer
    << t.text
    << t.orient
    << t.fontsize
    << ")";
  return d;
}
