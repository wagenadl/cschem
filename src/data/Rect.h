// Rect.h

#ifndef RECT_H

#define RECT_H

#include <QDebug>
#include "Point.h"
#include <QRectF>

struct Rect {
  Dim left;
  Dim top;
  Dim width;
  Dim height;
public:
  explicit Rect(Point tl=Point(), Point br=Point());
  Dim right() const { return left + width; }
  Dim bottom() const { return top + height; }
  QString toString() const;
  static Rect fromString(QString s, bool *ok=0);
  QRectF toMils() const;
  static Rect fromMils(QRectF const &p);
  bool operator==(Rect const &o) const;
  Rect &normalize();
  Rect &grow(Dim const &d); // grows by d/2 in all directions
  Rect &operator|=(Rect const &o);
  bool intersects(Rect const &o) const;
  bool contains(Rect const &o) const;
  bool contains(Point const &p) const;
  Rect translated(Point const &dxy) const;
  Point center() const;
};

QDebug operator<<(QDebug, Rect const &);



#endif
