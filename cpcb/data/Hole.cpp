// Hole.cpp

#include "Hole.h"
#include "Trace.h"
#include "FilledPlane.h"
#include "pi.h"
#include "Board.h"
#include "Pad.h"
#include <QTransform>

Hole::Hole() {
  fpcon = Layer::Invalid;
  via = false;
  square = false;
  noclear = false;
}

Rect Hole::boundingRect() const {
  Dim r = od/2;
  if (fpcon!=Layer::Invalid) 
    r += Board::padClearance(od,od) + Board::fpConOverlap();
  Rect rct(p - Point(r, r), p + Point(r, r));
  if (!rota.isCardinal() || slotlength.isPositive()) {
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

QPainterPath Hole::outlinePath(Layer l) const {
  QPainterPath path;
  double od_ = od.toMils();
  double sl = slotlength.toMils();
  if (square) {
    path.addRect(-od_/2 - sl/2, -od_/2, od_ + sl, od_);
  } else if (sl>0) {
    path.addRect(-sl/2, -od_/2, sl, od_);
    path.addEllipse(QPointF(-sl/2, 0), od_/2, od_/2);
    path.addEllipse(QPointF(sl/2, 0), od_/2, od_/2);
  } else {
    path.addEllipse(QPointF(0, 0), od_/2, od_/2);
  }
  if (l!=Layer::Invalid && l==fpcon) {
    double w = Board::fpConWidth(od, od).toMils();
    double dl = (Board::padClearance(od, od) + Board::fpConOverlap()).toMils();
    path.addRect(-od_/2 - dl - sl/2, -w/2, od_ + sl + 2*dl, w);
    path.addRect(-w/2, -od_/2 - dl, w, od_ + 2*dl);
  }
  double r = rota.degrees();
  if (r) {
    QTransform t;
    t.rotate(-r);
    path = t.map(path);
  }
  return path.translated(p.toMils());
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
  if (t.rota.degrees())
    s.writeAttribute("rot", t.rota.toString());
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
    << t.rota.toString()
    << ")";
  return d;
}

bool Hole::touches(class Hole const &other) const {
  if (!boundingRect().intersects(other.boundingRect()))
    return false;
  return outlinePath(fpcon).intersects(other.outlinePath(fpcon));
}
  

bool Hole::touches(class Pad const &pad) const {
  if (!(pad.layer==Layer::Top || pad.layer==Layer::Bottom))
    return false;
  if (!boundingRect().intersects(pad.boundingRect()))
    return false;
  return outlinePath(pad.layer).intersects(pad.outlinePath());
}
   
bool Hole::touches(Trace const &t) const {
  if (!(t.layer==Layer::Top || t.layer==Layer::Bottom))
    return false;
  if (!t.touches(boundingRect()))
    return false;
  return outlinePath(t.layer).intersects(t.outlinePath());
}

bool Hole::touches(FilledPlane const &fp) const {
  if (fpcon==fp.layer) {
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

void Hole::freeRotate(FreeRotation const &degcw, Point const &p0) {
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
  Point dp = Point(slotlength/2*rota.cos(), slotlength/2*rota.sin());
  return Segment(p-dp, p+dp);
}

  
