// Text.cpp

#include "Text.h"
#include "SimpleFont.h"

Text::Text() {
}

Rect Text::boundingRect() const {
  qDebug() << "Text::boundingRect - not yet correct";
  // must take orientation into account
  SimpleFont const &sf(SimpleFont::instance());
  Dim w(fontsize*sf.width(text)/sf.baseSize());
  Dim asc(fontsize*sf.ascent()/sf.baseSize());
  Dim desc(fontsize*sf.descent()/sf.baseSize());
  bool efflip = orient.flip ^ layer==Layer::Bottom;
  if (efflip)
    w = -w;
  switch (orient.rot & 3) {
  case 0: return Rect(p + Point(Dim(), desc), p + Point(w, -asc));
  case 1: return Rect(p + Point(asc, Dim()), p + Point(desc, w));
  case 2: return Rect(p + Point(Dim(), -desc), p + Point(-w, asc));
  case 3: return Rect(p + Point(-asc, Dim()), p + Point(-desc, -w));
  }
  return Rect(); // not executed 
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
    t.layer = Layer(a.value("l").toInt());
  else
    t.layer = Layer::Invalid;
  s.skipCurrentElement();
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
