// Segment.h

#ifndef SEGMENT_H

#define SEGMENT_H

#include "Rect.h"
#include "Fraction.h"

class Segment {
public:
  Point p1, p2;
public:
  Segment() {}
  Segment(Point a, Point b): p1(a), p2(b) {}
  Rect boundingRect() const;
  bool onP1(Point p, Dim mrg=Dim()) const;
  // ONP1(p, mrg) returns true if P is close enough to P1.
  // Close enough means within a distance MRG.
  bool onP2(Point p, Dim mrg=Dim()) const;
  bool onSegment(Point p, Dim mrg=Dim()) const;
  bool betweenEndpoints(Point p, Dim mrg=Dim()) const;
  // On the segment, but not on either endpoint
  std::optional<Point> intersection(Segment const &t) const;
  // The result may not be within the segment
  // Returns nil for parallel segments, even if collinear
  bool collinear(Segment const &t, Dim lmrg=Dim()) const;
  bool parallel(Segment const &t) const;
  Point projectionOntoLine(Point p) const { return _projection(p, false); }
  double angle(Segment const &t) const; // putting t after us. angle [-pi,+pi).
  double angle() const; // our angle wrt +ve x, [-pi,+pi).
  bool intersects(Segment const &t) const; // intersection within end points
  bool intersects(Rect r) const;
  bool projectsWithin(Point p, Dim mrg=Dim()) const;
  Dim distanceToLine(Point p) const { return _distance(p, false); }
  Dim distanceToSegment(Point p) const { return _distance(p, true); }
  Point nearestPoint(Point p) const { return _projection(p, true); }
protected:
  Fraction projectionCoefficient(Point p) const;
  Point _projection(Point p, bool constrain) const;  
  Dim _distance(Point p, bool constrain) const;
};

QDebug operator<<(QDebug, Segment const &);

#endif
