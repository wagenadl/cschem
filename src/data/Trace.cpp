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
  t = Trace();
  auto a = s.attributes();
  t.p1 = Point::fromString(a.value("p1").toString(), &ok);
  if (ok)
    t.p2 = Point::fromString(a.value("p2").toString(), &ok);
  if (ok)
    t.width = Dim::fromString(a.value("w").toString(), &ok);
  if (ok)
    t.layer = Layer(a.value("l").toInt());
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

Point Trace::intersectionWith(class Trace const &t, bool *ok) const {
  if (layer != t.layer) {
    if (ok)
      *ok = false;
    return Point();
  }

  /* This implementation is really primitive; it ignores trace width */
  /* Mathematical idea: represent us as p1 + a dp where a in [0, 1] and
     other trace as p1' + a' dp' (where a' in [0, 1]). Find intersection.
     We have x = x1 + a dx = x1' + a' dx' and y = y1 + a dy = y1' + a' dy'.
     Solve for a and a' and check if they are in (0,1).
     Write as:
     (1)    dx a - dx' a' = x1' - x1
     (2)    dy a - dy' a' = y1' - y1
     Multiply (1) by dy and (2) by dx and subtract:
            0 - (dx' dy  -  dy' dx) a' = dy (x1' - x1) - dx (y1' - y1)
     to find a', and multiple (1) by dy' and (2) by dx' and subtract:
            (dx dy' - dy dx') a = dy' (x1' - x1) - dx' (y1' - y1)
     to find a.
  */
  
  double x1 = p1.x.toMils();
  double y1 = p1.y.toMils();
  double dx = p2.x.toMils() - x1;
  double dy = p2.y.toMils() - y1;
  double x1_ = t.p1.x.toMils();
  double y1_ = t.p1.y.toMils();
  double dx_ = t.p2.x.toMils() - x1_;
  double dy_ = t.p2.y.toMils() - y1_;
  double a = (dy_*(x1_-x1) - dx_*(y1_-y1)) / (dx*dy_ - dy*dx_);
  double a_ = -(dy*(x1_-x1) - dx*(y1_-y1)) / (dx_*dy - dy_*dx);
  if (a>=0 && a<=1 && a_>=0 && a_<=1) {
    if (ok)
      *ok = true;
    return Point(Dim::fromMils(x1 + a*dx), Dim::fromMils(y1 + a*dy));
  } else {
    if (ok)
      *ok = false;
    return Point();
  }
}
