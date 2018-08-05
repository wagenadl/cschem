// Arc.h

#ifndef ARC_H

#define ARC_H

#include "Rect.h"
#include "Layer.h"
#include <QXmlStreamReader>
#include <QDebug>

class Arc {
public:
  enum class Extent {
    Invalid,
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
  int angle; // degrees
  /* If rot=0, arc is centred around 12 o'clock, unless angle is negative,
     in which case arc runs clockwise from 12 o'clock. */
  int rot; // as in Orient
  Layer layer;
public:
  Arc();
  void rotateCW();
  void rotateCCW();
  void flipLeftRight();
  void flipUpDown();
  void setLayer(Layer);
  Rect boundingRect() const;
  bool onEdge(Point p, Dim mrg) const;
  Extent extent() const; // only valid if set the old way
  void setExtent(Extent);
};

QDebug operator<<(QDebug, Arc const &);
QXmlStreamWriter &operator<<(QXmlStreamWriter &, Arc const &);
QXmlStreamReader &operator>>(QXmlStreamReader &, Arc &);


#endif
