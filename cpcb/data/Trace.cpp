// Trace.cpp

#include "Trace.h"
#include <cmath>
#include <QTransform>
#include "FilledPlane.h"
#include "pi.h"

Trace::Trace() {
  layer = Layer::Invalid;
  noclear = false;
}

QXmlStreamWriter &operator<<(QXmlStreamWriter &s, Trace const &t) {
  s.writeStartElement("trace");
  s.writeAttribute("p1", t.p1.toString());
  s.writeAttribute("p2", t.p2.toString());
  s.writeAttribute("w", t.width.toString());
  s.writeAttribute("l", QString::number(int(t.layer)));
  if (t.noclear)
    s.writeAttribute("noclear", "1");
  s.writeEndElement();
  return s;
}
  
QXmlStreamReader &operator>>(QXmlStreamReader &s, Trace &t) {
  bool ok;
  t = Trace();
  auto a = s.attributes();
  t.p1 = Point::fromString(a.value("p1").toString(), &ok);
  if (ok)
    t.p2 = Point::fromString(a.value("p2").toString(), &ok);
  if (ok)
    t.width = Dim::fromString(a.value("w").toString(), &ok);
  t.layer = Layer(a.value("l").toInt());
  t.noclear = a.value("noclear").toInt() != 0;
  s.skipCurrentElement();
  return s;
}

QDebug operator<<(QDebug d, Trace const &t) {
  d << "Trace(" << t.p1
    << t.p2
    << t.width
    << t.layer
    << (t.noclear ? "noclear" : "")
    << ")";
  return d;
}

Rect Trace::boundingRect() const {
  return Segment::boundingRect().grow(width);
}

QPainterPath Trace::outlinePath() const {
  Point dp = p2 - p1;
  Point px = Point(p1.distance(p2), Dim());
  double angle = std::atan2(dp.y.toMils(), dp.x.toMils());
  QPainterPath path;
  double w = width.toMils();
  path.addEllipse(QPoint(0,0), w/2, w/2);
  path.addEllipse(px.toMils(), w/2, w/2);
  path.addRect(0, -w/2, px.x.toMils(), w);
  QTransform t; t.rotate(angle*180/PI);
  return t.map(path).translated(p1.toMils());
}


bool Trace::onP1(Point p, Dim mrg) const {
  return Segment::onP1(p, mrg + width/2);
}

bool Trace::onP2(Point p, Dim mrg) const {
  return Segment::onP2(p, mrg + width/2);
}

bool Trace::onSegment(Point p, Dim mrg) const {
  return Segment::onSegment(p, mrg + width/2);
}

bool Trace::touches(FilledPlane const &fp) const {
  if (!noclear)
    return false;
  if (layer != fp.layer)
    return false;
  if (!boundingRect().intersects(fp.boundingRect()))
    return false;
  QPainterPath pp;
  pp.addPolygon(fp.perimeter.toMils());
  bool got = pp.intersects(outlinePath());
  //  if (fp.layer == Layer::Bottom)
  //qDebug() << "trace touch fp" << got;
  return got;
}

std::optional<Point> Trace::touchPoint(Trace const &t) const {
  if (layer != t.layer)
    return std::optional<Point>();
  if (!boundingRect().intersects(t.boundingRect()))
    return std::optional<Point>();
  if (!outlinePath().intersects(t.outlinePath()))
    return std::optional<Point>();
  
  if (onP1(t.p1) || onP1(t.p2))
    return p1;
  if (onP2(t.p1) || onP2(t.p2))
    return p2;

  Point intersect;
  bool got = intersects(t, &intersect); // this may not yield true, but if it doesn't
  // ... it should still be close enough
  Dim w = max(width, t.width);
  if (onP1(intersect, w/2)) 
    return p1;
  else if (onP2(intersect, w/2))
    return p2;
  else // if (got)
    return intersect;
  // 
}

bool Trace::touches(Trace const &t) const {
  if (touchPoint(t))
    return true;
  else
    return false;
}

bool Trace::operator==(Trace const &t) const {
  return layer==t.layer && width==t.width && p1==t.p1 && p2==t.p2;
}

bool Trace::touches(Rect r) const {
  r.grow(width);
  return intersects(r);
}
