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
  FreeRotation rota; /*  0: centered around +ve x (right)
                        90: centered around +ve y (down)
                       180: centered around -ve x (left)
                       270: centered around -ve y (up)
                     */ 
  Layer layer;
public:
  Arc();
  void freeRotate(FreeRotation const &degcw, Point const &p0);
  void rotateCW(); // around center
  void rotateCCW(); // around center
  void rotateCW(Point const &p0);
  void rotateCCW(Point const &p0);
  void flipLeftRight();
  void flipLeftRight(Dim x);
  void flipUpDown(); // around our center
  void flipUpDown(Dim y); // around y
  void setLayer(Layer);
  Rect boundingRect() const;
  bool onEdge(Point p, Dim mrg) const;
  void setExtent(Extent); // deprecated: use angle and rota instead
};

QDebug operator<<(QDebug, Arc const &);
QXmlStreamWriter &operator<<(QXmlStreamWriter &, Arc const &);
QXmlStreamReader &operator>>(QXmlStreamReader &, Arc &);


#endif
