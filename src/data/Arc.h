// Arc.h

#ifndef ARC_H

#define ARC_H

#include "Point.h"
class Arc {
public:
  enum class Extent {
    Full,
    LeftHalf,
    RightHalf,
    TopHalf,
    BottomHalf,
    TLQuadrant,
    TRQuadrant,
    BRQuadrant,
    BLQuadrant
  };
public:
  Point center;
  Dim radius; 
  Dim linewidth;
  Extent extent;
  Layer layer;
public:
  Arc();
  void rotateCW();
  void rotateCCW();
  void flip();
  void setLayer(Layer);
  Rect boundingRect() const;
  bool onEdge(Point p, Dim mrg) const;
};

QDebug operator<<(QDebug, Arc const &);
QXmlStreamWriter &operator<<(QXmlStreamWriter &, Arc const &);
QXmlStreamReader &operator>>(QXmlStreamReader &, Arc &);


#endif
