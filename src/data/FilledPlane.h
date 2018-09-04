// FilledPlane.h

#ifndef FILLEDPLANE_H

#define FILLEDPLANE_H

#include "Polyline.h"
#include "Layer.h"
#include "Rect.h"

#include <QXmlStreamWriter>
#include <QDebug>

class FilledPlane {
public:
  Polyline perimeter;
  Layer layer;
public:
  FilledPlane() { layer=Layer::Invalid; }
  bool isValid() const { return perimeter.size()>2 && layer!=Layer::Invalid; }
  Rect boundingRect() const;
  bool contains(Point p, Dim mrg=Dim()) const;
  bool touches(FilledPlane const &) const;
};

QDebug operator<<(QDebug, FilledPlane const &);
QXmlStreamWriter &operator<<(QXmlStreamWriter &, FilledPlane const &);
QXmlStreamReader &operator>>(QXmlStreamReader &, FilledPlane &);

#endif
