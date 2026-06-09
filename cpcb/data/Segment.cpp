// Segment.cpp

#include "Segment.h"
#include "pi.h"
#include <stdint.h>

Rect Segment::boundingRect() const {
  Rect r = Rect(p1, p2);
  return r;
}

bool Segment::onP1(Point p, Dim mrg) const {
  return p.distance(p1) <= mrg;
}

bool Segment::onP2(Point p, Dim mrg) const {
  return p.distance(p2) <= mrg;
}

Fraction Segment::projectionCoefficient(Point p) const {
  // returns number a such that p1 + a (p2 - p1) is the orthogonal projection of p
  // onto the segment
  if (p1 == p2)
    return Fraction();
  Point dp = p2 - p1;
  qint64 a = (p - p1).innerProduct(dp);
  qint64 b = dp.innerProduct(dp);
  return Fraction(a, b);
}

Dim Segment::_distance(Point p, bool constrain) const {
  Point pnear = _projection(p, constrain);
  return p.distance(pnear);
}


Point Segment::_projection(Point p, bool constrain) const {
  Fraction frc(projectionCoefficient(p));
  if (constrain) {
    if (frc.num <= 0)
      return p1;
    else if (frc.num >= frc.denom)
      return p2;
  }
  
  double x = (p2 - p1).x.raw();
  double y = (p2 - p1).y.raw();
  double n = frc.num;
  if (fabs(x*n) < 4e18 && fabs(y*n) < 4e18) {
    // not about to run out of space in int64_t.
    return p1 + (p2-p1) * frc.num / frc.denom;
  } else {
    // about to run out of space, therefore no risk of losing precision
    int64_t n_ = frc.num / 256;
    int64_t d = frc.denom / 256;
    return p1 + (p2-p1) * n_ / d;
  }
}

bool Segment::betweenEndpoints(Point p, Dim mrg) const {
  if (onP1(p, mrg))
    return false;
  if (onP2(p, mrg))
    return false;
  return onSegment(p, mrg / 2);
}

bool Segment::onSegment(Point p, Dim mrg) const {
  return distanceToSegment(p) <= mrg;
}

bool Segment::parallel(Segment const &t) const {
  return !!intersection(t);
}

bool Segment::collinear(Segment const &t, Dim lmrg) const {
  if (intersection(t))
    return false;
  return distanceToLine(t.p1) < lmrg;
}
  

std::optional<Point> Segment::intersection(Segment const &t) const {
  /* Mathematical idea: represent us as p1 + a dp where a in [0, 1] and
     other segment as p1' + a' dp' (where a' in [0, 1]). Find intersection.
     We have x = x1 + a dx = x1' + a' dx' and y = y1 + a dy = y1' + a' dy'.
     Solve for a and a'.
     Write as:
     (1)    dx a - dx' a' = x1' - x1
     (2)    dy a - dy' a' = y1' - y1
     Multiply (1) by dy and (2) by dx and subtract:
            0 - (dx' dy  -  dy' dx) a' = dy (x1' - x1) - dx (y1' - y1)
     to find a':
       a' = - [dy (x1' - x1) - dx (y1' - y1)] / (dx' dy  -  dy' dx)
     And multiply (1) by dy' and (2) by dx' and subtract:
            (dx dy' - dy dx') a = dy' (x1' - x1) - dx' (y1' - y1)
     to find a:
       a = [dy' (x1' - x1) - dx' (y1' - y1)] / (dx dy' - dy dx')
  */
  
  double x1 = p1.x.toMils();
  double y1 = p1.y.toMils();
  double dx = p2.x.toMils() - x1;
  double dy = p2.y.toMils() - y1;
  double x1_ = t.p1.x.toMils();
  double y1_ = t.p1.y.toMils();
  double dx_ = t.p2.x.toMils() - x1_;
  double dy_ = t.p2.y.toMils() - y1_;
  double nrm = dx*dy_ - dy*dx_;
  if (fabs(nrm)<1e-6)
    return std::optional<Point>();
  
  double a = (dy_*(x1_-x1) - dx_*(y1_-y1)) / nrm;
  return Point(Dim::fromMils(x1 + a*dx), Dim::fromMils(y1 + a*dy));
}

bool Segment::intersects(Segment const &t) const {
  double x1 = p1.x.toMils();
  double y1 = p1.y.toMils();
  double dx = p2.x.toMils() - x1;
  double dy = p2.y.toMils() - y1;
  double x1_ = t.p1.x.toMils();
  double y1_ = t.p1.y.toMils();
  double dx_ = t.p2.x.toMils() - x1_;
  double dy_ = t.p2.y.toMils() - y1_;
  double nrm = dx*dy_ - dy*dx_;
  if (fabs(nrm)<1e-6)
    return false; // parallel
  
  double a = (dy_*(x1_-x1) - dx_*(y1_-y1)) / nrm;
  if (a < 0 || a > 1)
    return false;
  double a_ = (dy*(x1_-x1) - dx*(y1_-y1)) / nrm;
  return a_ >= 0 && a_ <= 1;
}



double Segment::angle() const {
  return atan2(p2.y.toMils() - p1.y.toMils(),
               p2.x.toMils() - p1.x.toMils());
}

double Segment::angle(Segment const &t) const {
  double us = angle();
  double them = t.angle();
  double a = them - us;
  while (a >= PI)
    a -= 2*PI;
  while (a < -PI)
    a += 2*PI;
  return a;
}

bool Segment::intersects(Rect r) const {
  return r.contains(p1) || r.contains(p2)
    || intersects(Segment(r.topLeft(), r.topRight()))
    || intersects(Segment(r.topRight(), r.bottomRight()))
    || intersects(Segment(r.bottomRight(), r.bottomLeft()))
    || intersects(Segment(r.bottomLeft(), r.topLeft()));
}

bool Segment::projectsWithin(Point p, Dim mrg) const {
  return onSegment(projectionOntoLine(p), mrg);
}


QDebug operator<<(QDebug d, Segment const &t) {
  d << "Segment(" << t.p1 << t.p2 << ")";
  return d;
}
