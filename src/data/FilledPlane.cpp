// FilledPlane.cpp

#include "FilledPlane.h"

#include <QPolygonF>

Rect FilledPlane::boundingRect() const {
  if (!isValid())
    return Rect();
  Rect r(perimeter[0], perimeter[1]);
  for (Point const &p: perimeter)
    r |= p;
  return r;
}

QXmlStreamWriter &operator<<(QXmlStreamWriter &s, FilledPlane const &t) {
  s.writeStartElement("plane");
  s.writeAttribute("pp", t.perimeter.toString());
  s.writeAttribute("l", QString::number(int(t.layer)));
  s.writeEndElement();
  return s;
}
  
QXmlStreamReader &operator>>(QXmlStreamReader &s, FilledPlane &t) {
  t = FilledPlane();
  bool ok;
  auto a = s.attributes();
  t.layer = Layer(a.value("l").toInt(&ok));
  if (ok)
    t.perimeter = Polyline::fromString(a.value("pp").toString(), &ok);
  s.skipCurrentElement();
  return s;
}

QDebug operator<<(QDebug d, FilledPlane const &t) {
  d << "FilledPlane("
    << t.layer
    << t.perimeter.toMils()
    << ")";
  return d;
}

bool FilledPlane::contains(Point p, Dim mrg) const {
  return perimeter.contains(p, mrg);
}

bool FilledPlane::touches(FilledPlane const &fp) const {
  return layer==fp.layer
    && !perimeter.toMils().intersected(fp.perimeter.toMils()).isEmpty();
}
