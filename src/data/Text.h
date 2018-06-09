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
  void flip();
  void rotateCW();
  void rotateCCW();
  void setLayer(Layer l);
};

QDebug operator<<(QDebug, Text const &);
QXmlStreamWriter &operator<<(QXmlStreamWriter &, Text const &);
QXmlStreamReader &operator>>(QXmlStreamReader &, Text &);

#endif
