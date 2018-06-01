// Trace.h

#ifndef TRACE_H

#define TRACE_H

#include "Point.h"
#include "Layer.h"
#include <QXmlStreamReader>
#include <QDebug>

class Trace {
public:
  Layer layer;
  Point p1, p2;
  Dim width;
public:
  Trace();
  bool isValid() const { return layer!=Layer::Invalid; }
};

QDebug operator<<(QDebug, Trace const &);
QXmlStreamWriter &operator<<(QXmlStreamWriter &, Trace const &);
QXmlStreamReader &operator>>(QXmlStreamReader &, Trace &);

#endif
