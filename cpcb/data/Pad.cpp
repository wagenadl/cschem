// Pad.cpp

#include "Pad.h"
#include "Trace.h"
#include "FilledPlane.h"
#include "Board.h"
#include <QTransform>

Pad::Pad() {
  fpcon = false;
  noclear = false;
}


void Pad::freeRotate(int degcw, Point const &p0) {
  rota += degcw;
  if (rota==180) {
    rota = FreeRotation();
  } else if (rota==90 || rota==270) {
    Dim x = width;
    width = height;
    height = x;
    rota = FreeRotation();
  }
  p.freeRotate(degcw, p0);
}

void Pad::rotateCW() {
  rota += 90;
  if (rota==180) {
    rota = FreeRotation();
  } else if (rota==90 || rota==270) {
    Dim x = width;
    width = height;
    height = x;
    rota = FreeRotation();
  }
}

QXmlStreamWriter &operator<<(QXmlStreamWriter &s, Pad const &t) {
  s.writeStartElement("pad");
  s.writeAttribute("p", t.p.toString());
  s.writeAttribute("w", t.width.toString());
  s.writeAttribute("h", t.height.toString());
  s.writeAttribute("l", QString::number(int(t.layer)));
  s.writeAttribute("ref", t.ref);
  if (t.fpcon)
    s.writeAttribute("fp", "1");
  if (t.noclear)
    s.writeAttribute("noclear", "1");
  if (t.rota)
    s.writeAttribute("rot", QString::number(t.rota));
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
    t.layer = Layer(a.value("l").toInt());
  else
    t.layer = Layer::Invalid;
  t.fpcon = a.value("fp") != 0;
  t.noclear = a.value("noclear").toInt() != 0;
  t.ref = a.value("ref").toString();
  t.rota = FreeRotation(a.value("rot").toInt());
  s.skipCurrentElement();
  return s;
}

QDebug operator<<(QDebug d, Pad const &t) {
  d << "Pad(" << t.ref << t.p
    << t.width
    << t.height
    << t.layer
    << (t.fpcon ? "fp" : "")
    << (t.noclear ? "noclear" : "")
    << t.rota
    << ")";
  return d;
}

Rect Pad::boundingRect() const {
  Dim rx = width/2;
  Dim ry = height/2;
  if (fpcon)
    rx += Board::padClearance(width, height) + Board::fpConOverlap();
  Point dp(rx, ry);
  if (rota) {
    // I'm not at all sure this is correct
    Rect r(p, p);
    r |= p + dp.rotatedFreely(rota);
    r |= p + (-dp).rotatedFreely(rota);
    dp.y = -dp.y;
    r |= p + dp.rotatedFreely(rota);
    r |= p + (-dp).rotatedFreely(rota);
    return r;
  } else {
    return Rect(p - dp, p + dp);
  }
}

QPainterPath Pad::outlinePath() const {
  QPainterPath path;
  double w = width.toMils();
  double h = height.toMils();
  path.addRect(-w/2, -h/2, w, h);
  if (fpcon) {
    double w1 = Board::fpConWidth(width, height).toMils();
    double dl = (Board::padClearance(width, height)
                 + Board::fpConOverlap()).toMils();
    path.addRect(-w/2 - dl, -w1/2, w + 2*dl, w1);
    path.addRect(-w1/2, -h/2 - dl, w1, h + 2*dl);
  }
  int r = rota;
  if (r) {
    QTransform t;
    t.rotate(-r);
    path = t.map(path);
  }
  return path.translated(p.toMils());
}



bool Pad::touches(class Pad const &pad) const {
  if (pad.layer != layer)
    return false;
  if (!boundingRect().intersects(pad.boundingRect()))
    return false;

  return outlinePath().intersects(pad.outlinePath());
}

bool Pad::touches(class Trace const &t) const {
  if (t.layer != layer)
    return false;
  if (p==t.p1 || p==t.p2)
    return true;
  Dim diam = Dim::quadrature(width/2, height/2);
  if (t.onSegment(p, diam)) {
    // This is a slightly liberal interpretation of touching, actually, so
    // let's be a bit more careful. Following is not exact either,
    // but should be good enough in all but rather pathological cases.
    Dim min_r = width < height ? width/2 : height/2;
    return t.onSegment(p, min_r)
			|| t.onSegment(p + Point(width/2, height/2))
			|| t.onSegment(p + Point(-width/2, height/2))
			|| t.onSegment(p + Point(width/2, -height/2))
			|| t.onSegment(p + Point(-width/2, -height/2))
			|| t.onSegment(p + Point(width/2, Dim()))
			|| t.onSegment(p + Point(-width/2, Dim()))
			|| t.onSegment(p + Point(Dim(), height/2))
			|| t.onSegment(p + Point(Dim(), -height/2));
  }
  return false;
}

bool Pad::touches(FilledPlane const &fp) const {
  if (layer!=fp.layer)
    return false;
  if (noclear) {
    return fp.perimeter.contains(p);
    /* Warning: This is WRONG if an attached trace creates clearance */
  } else if (fpcon) {
    Dim r = width > height ? width/2 : height/2;
    return fp.perimeter.contains(p, r);
  } else {
    return false;
  }
}

void Pad::rotateCW(Point const &p0) {
  rotateCW();
  p.rotateCW(p0);
}

void Pad::flipLeftRight(Dim const &x0) {
  rota.flipLeftRight();
  p.flipLeftRight(x0);
}

void Pad::flipUpDown(Dim const &y0) {
  rota.flipUpDown();
  if (rota==180)
    rota = FreeRotation();
  p.flipUpDown(y0);
}
