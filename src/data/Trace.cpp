// Trace.cpp

#include "Trace.h"

Trace::Trace() {
  layer = Layer::Invalid;
}

QXmlStreamWriter &operator<<(QXmlStreamWriter &s, Trace const &t) {
  s.writeStartElement("trace");
  s.writeAttribute("p1", t.p1.toString());
  s.writeAttribute("p2", t.p2.toString());
  s.writeAttribute("w", t.width.toString());
  s.writeAttribute("l", QString::number(int(t.layer)));
  s.writeEndElement();
  return s;
}
  
QXmlStreamReader &operator>>(QXmlStreamReader &s, Trace &t) {
  bool ok;
  auto a = s.attributes();
  t.p1 = Point::fromString(a.value("p1").toString(), &ok);
  if (ok)
    t.p2 = Point::fromString(a.value("p2").toString(), &ok);
  else
    t.p2 = Point();
  if (ok)
    t.width = Dim::fromString(a.value("w").toString(), &ok);
  else
    t.width = Dim();
  if (ok)
    t.layer = Layer(a.value("layer").toInt());
  else
    t.layer = Layer::Invalid;
  s.skipCurrentElement();
  return s;
}

QDebug operator<<(QDebug d, Trace const &t) {
  d << "Trace(" << t.p1
    << t.p2
    << t.width
    << t.layer
    << ")";
  return d;
}

Rect Trace::boundingRect() const {
  Rect r = Rect(p1, p2).grow(width);
  return r;
}

bool Trace::onP1(Point p, Dim mrg) const {
  return p.distance(p1) < width/2 + mrg;
}

bool Trace::onP2(Point p, Dim mrg) const {
  return p.distance(p2) < width/2 + mrg;
}

bool Trace::onSegment(Point p, Dim mrg) const {
  Rect bb = boundingRect().grow(mrg*2);
  if (!bb.contains(p))
    return false; // if p is beyond the end points, don't accept it.
  auto sq = [](double x) { return x*x; };
  double x1 = p1.x.toMils();
  double y1 = p1.y.toMils();
  double x2 = p2.x.toMils();
  double y2 = p2.y.toMils();
  double x0 = p.x.toMils();
  double y0 = p.y.toMils();

  // According to wikipedia:
  // https://en.wikipedia.org/wiki/Distance_from_a_point_to_a_line
  // The distance between point p and line through p1 and p2 is given by
  // a/b where a and b are:
  double a = fabs((y2-y1)*x0 - (x2-x1)*y0 + x2*y1 - y2*x1);
  double b = sqrt(sq(y2-y1) + sq(x2-x1));
  double m = (width/2 + mrg).toMils();
  // Instead of testing a/b < m, I test a < m*b to avoid misery when p1==p2.
  return a < m*b;
}
