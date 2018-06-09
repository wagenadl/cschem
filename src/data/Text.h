// Text.h

#ifndef TEXT_H

#define TEXT_H

#include "Point.h"
#include "Layer.h"
#include "Orient.h"
#include "Rect.h"
#include <QXmlStreamReader>
#include <QDebug>

class Text {
public:
  Point p; // left, baseline of text
  Dim fontsize;
  Layer layer;
  Orient orient;
  QString text;
  bool isref; // this text object represents the "Ref." of the containing group
public:
  Text();
  bool isValid() const { return layer!=Layer::Invalid; }
  Rect boundingRect() const;
  void flipLeftRight(); // around our center
  void flipUpDown();
  void flipLeftRight(Dim x0); // around another point
  void flipUpDown(Dim y0);
  void rotateCW(); // around our center
  void rotateCCW();
  void rotateCW(Point const &); // around another point
  void rotateCCW(Point const &); // around another point
  void setLayer(Layer l);
};

QDebug operator<<(QDebug, Text const &);
QXmlStreamWriter &operator<<(QXmlStreamWriter &, Text const &);
QXmlStreamReader &operator>>(QXmlStreamReader &, Text &);

#endif
