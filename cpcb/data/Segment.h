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
  // ONSEGMENT(p, mrg) returns true if P is close enough to the line segment.
  // Close enough means within a distance MRG.
  // ONSEGMENT returns true even if ONP1 or ONP2 would return true as well.
  bool betweenEndpoints(Point p, Dim mrg=Dim()) const;
  // Like ONSEGMENT, but not true when on either endpoint
  bool intersects(Segment const &t, Point *intersection=0) const;
  // The returned intersection point is on the segment if the result is true,
  // otherwise it is on the line extended from the segment.
  Point projectionOntoSegment(Point p) const;
  // returns p1 or p2 if projection onto line would be outside of segment
  double angle(Segment const &t) const; // putting t after us. angle [-pi,+pi).
  double angle() const; // our angle wrt +ve x, [-pi,+pi).
  bool intersects(Rect r) const;
protected:
  Fraction projectionCoefficient(Point p) const;
};

QDebug operator<<(QDebug, Segment const &);

#endif
