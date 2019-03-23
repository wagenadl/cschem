// Segment.cpp

#include "Segment.h"
#include "pi.h"

Rect Segment::boundingRect() const {
  Rect r = Rect(p1, p2);
  return r;
}

bool Segment::onP1(Point p, Dim mrg) const {
  return p.distance(p1) < mrg;
}

bool Segment::onP2(Point p, Dim mrg) const {
  return p.distance(p2) < mrg;
}

Fraction Segment::projectionCoefficient(Point p) const {
  // returns number a such that p1 + a(p2-p1) is the orthogonal projection of p
  // onto the segment
  if (p1==p2)
    return Fraction();
  Point dp = p2 - p1;
  qint64 a = (p - p1).innerProduct(dp);
  qint64 b = dp.innerProduct(dp);
  return Fraction(a, b);
}

Point Segment::projectionOntoSegment(Point p) const {
  Fraction frc(projectionCoefficient(p));
  if (frc.num<=0)
    return p1;
  else if (frc.num>=frc.denom)
    return p2;
  else
    return p1 + (p2-p1)*frc.num / frc.denom;
}

bool Segment::betweenEndpoints(Point p, Dim mrg) const {
  if (!onSegment(p, mrg))
    return false;
  if (onP1(p, mrg))
    return false;
  if (onP2(p, mrg))
    return false;
  Fraction frc(projectionCoefficient(p));
  if (frc.num<=0)
    return false;
  if (frc.num>=frc.denom)
    return false;
  return true;
}  

bool Segment::onSegment(Point p, Dim mrg) const {
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
  double m = mrg.toMils();
  // Instead of testing a/b < m, I test a < m*b to avoid misery when p1==p2.
  return a < m*b;
}

bool Segment::intersects(Segment const &t, Point *intersection) const {
  bool res;
  Point p = intersectionWith(t, &res);
  if (intersection)
    *intersection = p;
  return res;
}

Point Segment::intersectionWith(class Segment const &t, bool *ok) const {
  /* Mathematical idea: represent us as p1 + a dp where a in [0, 1] and
     other segment as p1' + a' dp' (where a' in [0, 1]). Find intersection.
     We have x = x1 + a dx = x1' + a' dx' and y = y1 + a dy = y1' + a' dy'.
     Solve for a and a' and check if they are in (0,1).
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
  if (fabs(nrm)<1e-6) {
    // parallel; "intersects" iff directly overlapping
    if (onSegment(t.p1, Dim::fromMM(.001))) {
      if (ok)
	*ok = true;
      return t.p1;
    } else if (onSegment(t.p2, Dim::fromMM(.001))) {
      if (ok)
	*ok = true;
      return t.p2;
    } else if (t.onSegment(p1, Dim::fromMM(.001))) {
      if (ok)
	*ok = true;
      return p1;
    } else if (t.onSegment(p2, Dim::fromMM(.001))) {
      if (ok)
	*ok = true;
      return p2;
    } else {
      if (ok)
	*ok = false;
      return Point();
    }
  }
  double a = (dy_*(x1_-x1) - dx_*(y1_-y1)) / nrm;
  double a_ = (dy*(x1_-x1) - dx*(y1_-y1)) / nrm;
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

double Segment::angle(Segment const &t) const {
  double us = atan2(p2.y.toMils() - p1.y.toMils(),
		    p2.x.toMils() - p1.x.toMils());
  double them = atan2(t.p2.y.toMils() - t.p1.y.toMils(),
		      t.p2.x.toMils() - t.p1.x.toMils());
  double a = them - us;
  while (a>=PI)
    a -= 2*PI;
  while (a<-PI)
    a += 2*PI;
  return a;
}

Segment Segment::orthogonallyDisplaced(Dim d) const {
  double us = atan2(p2.y.toMils() - p1.y.toMils(),
		    p2.x.toMils() - p1.x.toMils());
  Point dxy(d*sin(us), d*cos(us));
  return Segment(p1 + dxy, p2 + dxy);
}

bool Segment::intersects(Rect r) const {
  return r.contains(p1) || r.contains(p2)
    || intersects(Segment(r.topLeft(), r.topRight()))
    || intersects(Segment(r.topRight(), r.bottomRight()))
    || intersects(Segment(r.bottomRight(), r.bottomLeft()))
    || intersects(Segment(r.bottomLeft(), r.topLeft()));
}

QDebug operator<<(QDebug d, Segment const &t) {
  d << "Segment(" << t.p1 << t.p2 << ")";
  return d;
}
