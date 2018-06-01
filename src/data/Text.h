// Text.h

#ifndef TEXT_H

#define TEXT_H

#include "Point.h"
#include "Layer.h"
#include "Orient.h"
#include <QXmlStreamReader>
#include <QDebug>

class Text {
public:
  Point p;
  Dim fontsize;
  Layer layer;
  Orient orient;
  QString text;
public:
  Text();
  bool isValid() const { return layer!=Layer::Invalid; }
};

QDebug operator<<(QDebug, Text const &);
QXmlStreamWriter &operator<<(QXmlStreamWriter &, Text const &);
QXmlStreamReader &operator>>(QXmlStreamReader &, Text &);

#endif
