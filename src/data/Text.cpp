// Text.cpp

#include "Text.h"
#include "SimpleFont.h"

Text::Text() {
  groupaffiliation = 0;
}

void Text::setGroupAffiliation(int id) {
  groupaffiliation = id;
}

int Text::groupAffiliation() const {
  return groupaffiliation;
}

Rect Text::boundingRect() const {
  SimpleFont const &sf(SimpleFont::instance());
  Dim w(fontsize*sf.width(text)/sf.baseSize());
  Dim asc(fontsize*sf.ascent()/sf.baseSize());
  Dim desc(fontsize*sf.descent()/sf.baseSize());
  bool efflip = orient.flip ^ (layer==Layer::Bottom);
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

void Text::flipLeftRight() {
  flipLeftRight(boundingRect().center().x);
}

void Text::flipLeftRight(Dim x0) {
  Point ctarget = boundingRect().center().flippedLeftRight(x0);
  qDebug() << "text:flipleftright x0=" << x0 << "p=" << p
	   << "br=" << boundingRect()
	   << "center=" << boundingRect().center()
	   << "target=" << ctarget;
  orient.flip = !orient.flip;
  if (orient.rot&1)
    orient.rot ^= 2;
  Point c1 = boundingRect().center();
  qDebug() << "new center" << c1;
  p += ctarget - c1; // shift it back so center is maintained
  qDebug() << "new p" << p;
}
void Text::flipUpDown() {
  flipUpDown(boundingRect().center().y);
}

void Text::flipUpDown(Dim y0) {
  Point ctarget = boundingRect().center().flippedUpDown(y0);
  orient.flip = !orient.flip;
  if ((orient.rot&1) == 0)
    orient.rot ^= 2;
  Point c1 = boundingRect().center();
  p += ctarget - c1; // shift it back so center is maintained
}

void Text::rotateCCW() {
  rotateCCW(boundingRect().center());
}

void Text::rotateCCW(Point const &p0) {
  Point ctarget = boundingRect().center().rotatedCCW(p0);
  orient.rot = (orient.rot + 1) & 3;
  Point c1 = boundingRect().center();
  p += ctarget - c1; // shift it back so center is maintained
}

void Text::rotateCW() {
  rotateCW(boundingRect().center());
}

void Text::rotateCW(Point const &p0) {
  Point ctarget = boundingRect().center().rotatedCW(p0);
  orient.rot = (orient.rot + 1) & 3;
  Point c1 = boundingRect().center();
  p += ctarget - c1; // shift it back so center is maintained
}


void Text::setLayer(Layer l1) {
  Point c0 = boundingRect().center();
  layer = l1;
  Point c1 = boundingRect().center();
  p += c0 - c1; // shift it back so center is maintained
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
    << t.groupAffiliation()
    << ")";
  return d;
}
