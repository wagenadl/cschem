// Text.cpp

#include "Text.h"
#include "SimpleFont.h"

Text::Text() {
  groupaffiliation = -1;
  flip = false;
}

void Text::setGroupAffiliation(int id) {
  groupaffiliation = id;
}

int Text::groupAffiliation() const {
  return groupaffiliation;
}

Rect Text::boundingRect() const {
  SimpleFont const &sf(SimpleFont::instance());
  double scl = sf.scaleFactor(fontsize);
  Dim w(scl*sf.width(text));
  Dim asc(scl*sf.ascent());
  Dim desc(scl*sf.descent());
  bool efflip = flip ^ (layer==Layer::Bottom || layer==Layer::BSilk);
  FreeRotation effrot = rota;//efflip ? -rota : rota;
  if (efflip)
    w = -w;
  Rect r0(p + Point(Dim(), desc), p + Point(w, -asc));
  if (effrot.degrees()) {
    Rect r(p, p);
    r|= Point(r0.left, r0.top).rotatedFreely(effrot, p);
    r|= Point(r0.right(), r0.top).rotatedFreely(effrot, p);
    r|= Point(r0.left, r0.bottom()).rotatedFreely(effrot, p);
    r|= Point(r0.right(), r0.bottom()).rotatedFreely(effrot, p);
    return r;
  } else {
    return r0;
  }
}

void Text::flipLeftRight() {
  flipLeftRight(boundingRect().center().x);
}

void Text::flipLeftRight(Dim x0, bool nottext) {
  Point ctarget = boundingRect().center().flippedLeftRight(x0);
  // qDebug() << "text:flipleftright x0=" << x0 << "p=" << p
  //          << "br=" << boundingRect()
  //          << "center=" << boundingRect().center()
  //          << "target=" << ctarget;
  if (!nottext) {
    flip = !flip;
    rota = -rota;
  }
  Point c1 = boundingRect().center();
  //  qDebug() << "new center" << c1;
  p += ctarget - c1; // shift it back so center is maintained
  //  qDebug() << "new p" << p;
}

void Text::flipUpDown() {
  flipUpDown(boundingRect().center().y);
}

void Text::flipUpDown(Dim y0, bool nottext) {
  // qDebug() << "text::flipupdown" << *this << y0;
  Point ctarget = boundingRect().center().flippedUpDown(y0);
  if (!nottext) {
    rotateCCW();
    flipLeftRight();
    rotateCW();
  }
  Point c1 = boundingRect().center();
  p += ctarget - c1; // shift it back so center is maintained
  // qDebug() << "  now" << *this;
}


void Text::freeRotate(FreeRotation const &degcw, Point const &p0) {
  Point ctarget = boundingRect().center().rotatedFreely(degcw, p0);
  rota += degcw;
  p.freeRotate(degcw, p0);
  p += ctarget - boundingRect().center();
}

void Text::rotateCCW() {
  rotateCCW(boundingRect().center());
}

void Text::rotateCCW(Point const &p0, bool nottext) {
  Point ctarget = boundingRect().center().rotatedCCW(p0);
  if (!nottext)
    rota -= 90;
  Point c1 = boundingRect().center();
  p += ctarget - c1; // shift it back so center is maintained
}

void Text::rotateCW() {
  rotateCW(boundingRect().center());
}

void Text::rotateCW(Point const &p0, bool nottext) {
  Point ctarget = boundingRect().center().rotatedCW(p0);
  if (!nottext)
    rota += 90;
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
  s.writeAttribute("rot", t.rota.toString());
  if (t.flip)
    s.writeAttribute("flip", "1");
  s.writeAttribute("text", t.text);
  s.writeEndElement();
  return s;
}
  
QXmlStreamReader &operator>>(QXmlStreamReader &s, Text &t) {
  bool ok;
  auto a = s.attributes();
  t.p = Point::fromString(a.value("p").toString(), &ok);
  t.fontsize = Dim::fromString(a.value("fs").toString(), &ok);
  if (a.hasAttribute("ori")) {
    QStringList bits = a.value("ori").toString().split(" ");
    t.rota = FreeRotation(bits[0].toInt() * 90);
    t.flip = bits[1].toInt() ? true : false;
  } else {
    t.rota = FreeRotation(a.value("rot").toInt());
    t.flip = a.value("flip").toInt() ? true : false;
  }
  t.text = a.value("text").toString();
  t.layer = Layer(a.value("l").toInt());
  s.skipCurrentElement();
  return s;
}

QDebug operator<<(QDebug d, Text const &t) {
  d << "Text(" << t.p
    << t.layer
    << t.text
    << t.rota.degrees()
    << t.flip
    << t.fontsize
    << t.groupAffiliation()
    << ")";
  return d;
}
