// Text.h

#ifndef TEXT_H

#define TEXT_H

#include "Point.h"
#include "Layer.h"
#include "Rect.h"
#include "FreeRotation.h"
#include <QXmlStreamReader>
#include <QDebug>

class Text {
public:
  Point p; // left, baseline of text
  Dim fontsize;
  Layer layer;
  FreeRotation rota;
  bool flip;
  QString text;
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
  void setGroupAffiliation(int);
  int groupAffiliation() const;
private:
  int groupaffiliation; // this text object represents the "Ref." of the
  // ID'd group. ID is in terms of the containing group
};

QDebug operator<<(QDebug, Text const &);
QXmlStreamWriter &operator<<(QXmlStreamWriter &, Text const &);
QXmlStreamReader &operator>>(QXmlStreamReader &, Text &);

#endif
