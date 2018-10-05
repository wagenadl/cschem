// Arc.h

#ifndef ARC_H

#define ARC_H

#include "Rect.h"
#include "Layer.h"
#include "FreeRotation.h"
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
  int angle; // span, degrees
  FreeRotation rota;
  Layer layer;
public:
  Arc();
  void rotateFreely(int);
  void rotateCW();
  void rotateCCW();
  void flipLeftRight();
  void flipUpDown(); // around our center
  void flipUpDown(Dim y); // around y
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
