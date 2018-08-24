// Segment.h

#ifndef SEGMENT_H

#define SEGMENT_H

#include "Rect.h"

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
  Point intersectionWith(Segment const &t, bool *ok=0) const;
};

#endif
