// Hole.cpp

#include "Hole.h"
#include "Trace.h"
#include "FilledPlane.h"

Hole::Hole() {
  fpcon = Layer::Invalid;
  via = false;
  square = false;
  noclear = false;
}

Rect Hole::boundingRect() const {
  Dim r = od/2;
  Rect rct(p - Point(r, r), p + Point(r, r));
  if (rota%90!=0 || slotlength.isPositive()) {
    Dim dx = slotlength/2;
    Point nw(-r-dx, -r);
    Point ne(r+dx, -r);
    Point sw(-r-dx, r);
    Point se(r+dx, r);
    rct |= p + nw.rotatedFreely(rota);
    rct |= p + ne.rotatedFreely(rota);
    rct |= p + sw.rotatedFreely(rota);
    rct |= p + se.rotatedFreely(rota);
  }
  return rct;
}

QXmlStreamWriter &operator<<(QXmlStreamWriter &s, Hole const &t) {
  s.writeStartElement("hole");
  s.writeAttribute("p", t.p.toString());
  s.writeAttribute("id", t.id.toString());
  s.writeAttribute("od", t.od.toString());
  if (t.square)
    s.writeAttribute("sq", "1");
  if (t.via)
    s.writeAttribute("via", "1");
  s.writeAttribute("ref", t.ref);
  if (t.fpcon != Layer::Invalid)
    s.writeAttribute("fp", QString::number(int(t.fpcon)));
  if (t.noclear)
    s.writeAttribute("noclear", "1");
  if (t.slotlength.isPositive())
    s.writeAttribute("sl", t.slotlength.toString());
  if (t.rota)
    s.writeAttribute("rot", QString::number(t.rota));
  s.writeEndElement();
  return s;
}
  
QXmlStreamReader &operator>>(QXmlStreamReader &s, Hole &t) {
  t = Hole();
  bool ok;
  auto a = s.attributes();
  t.ref = a.value("ref").toString();
  t.p = Point::fromString(a.value("p").toString(), &ok);
  t.square = a.value("sq").toInt();
  t.via = a.value("via").toInt();
  t.id = Dim::fromString(a.value("id").toString(), &ok);
  t.od = Dim::fromString(a.value("od").toString(), &ok);
  t.fpcon = Layer(a.value("fp").toInt());
  t.noclear = a.value("noclear").toInt() != 0;
  t.slotlength = Dim::fromString(a.value("sl").toString());
  t.rota = FreeRotation(a.value("rot").toInt());
  s.skipCurrentElement();
  return s;
}

QDebug operator<<(QDebug d, Hole const &t) {
  d << "Hole("
    << t.ref
    << t.p
    << t.id
    << t.od
    << (t.square ? "square" : t.via ? "via" : "circ")
    << int(t.fpcon)
    << (t.noclear ? "noclear" : "")
    << t.slotlength
    << t.rota
    << ")";
  return d;
}

bool Hole::touches(Trace const &t) const {
  if (!(t.layer==Layer::Top || t.layer==Layer::Bottom))
    return false;
  if (!t.touches(boundingRect()))
    return false;
  if (p==t.p1 || p==t.p2)
    return true;
  if (t.onSegment(p, od/2))
    return true;
  if (square) {
    Segment t1;
    t1.p1 = t.p1.rotatedFreely(-rota, p);
    t1.p2 = t.p2.rotatedFreely(-rota, p);
    Point dxy(slotlength/2+od/2,od/2);
    Rect r0(p - dxy, p + dxy);
    return t1.intersects(r0)
      || t1.orthogonallyDisplaced(-t.width/2).intersects(r0)
      || t1.orthogonallyDisplaced(t.width/2).intersects(r0);
  } else {
    if (slotlength.isPositive()) {
      constexpr double PI = 4*atan(1.0);
      Trace me;
      me.layer = t.layer;
      me.width = od;
      Point dxy(cos(PI*rota/180)*slotlength/2,
		sin(PI*rota/180)*slotlength/2);
      me.p1 = p + dxy;
      me.p2 = p - dxy;
      return t.touches(me);
    } else {
      return false;
    }
  }
}

bool Hole::touches(FilledPlane const &fp) const {
  if (noclear || fpcon==fp.layer) {
    if (fp.perimeter.contains(p, od/2))
      return true;
    return false; // this is not quite good enough
  } else {
    return false;
  }
}

void Hole::rotateCW(Point const &p0) {
  rota += 90;
  p.rotateCW(p0);
}

void Hole::freeRotate(int degcw, Point const &p0) {
  rota += degcw;
  p.freeRotate(degcw, p0);
}

void Hole::flipLeftRight(Dim const &x0) {
  rota.flipLeftRight();
  p.flipLeftRight(x0);
}

void Hole::flipUpDown(Dim const &y0) {
  rota.flipUpDown();
  p.flipUpDown(y0);
}

bool Hole::isSlot() const {
  return slotlength.isPositive();
}

Segment Hole::slotEnds() const {
  constexpr double PI = 4*atan(1);
  double phi = rota*PI/180;
  Point dp = Point(slotlength/2*cos(phi), slotlength/2*sin(phi));
  return Segment(p-dp, p+dp);
}

  
