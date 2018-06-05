// Rect.cpp

#include "Rect.h"

Rect::Rect(Point tl, Point br) {
  left=tl.x;
  top=tl.y;
  width=br.x-tl.x;
  height=br.y-tl.y;
  normalize();
}

QString Rect::toString() const {
  return left.toString() + " " + top.toString()
     + " " + width.toString() + " " + height.toString();
}

Rect Rect::fromString(QString s, bool *ok) {
  Rect r;
  bool ok1 = false;
  QStringList l = s.split(" ");
  if (l.size()==4) {
    r.left = Dim::fromString(l[0], &ok1);
    if (ok1)
      r.top = Dim::fromString(l[1], &ok1);
    if (ok1)
      r.width = Dim::fromString(l[2], &ok1);
    if (ok1)
      r.height = Dim::fromString(l[3], &ok1);
  }
  if (ok)
    *ok = ok1;
  return r;
}

Rect &Rect::normalize() {
  if (width.isNegative()) {
    left += width;
    width = -width;
  }
  if (height.isNegative()) {
    top += height;
    height = -height;
  }
  return *this;
}

Rect &Rect::grow(Dim const &d) {
  left -= d/2;
  top -= d/2;
  width += d;
  height += d;
  return *this;
}

QRectF Rect::toMils() const {
  return QRectF(QPointF(left.toMils(), top.toMils()),
		QSizeF(width.toMils(), height.toMils()));
}

Rect Rect::fromMils(QRectF const &p) {
  return Rect(Point::fromMils(p.topLeft()), Point::fromMils(p.bottomRight()));
}

QDebug operator<<(QDebug d, Rect const &r) {
  d << r.left.toInch() << r.top.toInch()
    << r.width.toInch() << r.height.toInch();
  return d;
};

Rect &Rect::operator|=(Rect const &o) {
  if (width.isNull() || height.isNull()) {
    *this = o;
    return *this;
  } else if (o.width.isNull() || o.height.isNull()) {
    return *this;
  }
  Dim right = left + width;
  Dim bottom = top + height;
  Dim oright = o.left + o.width;
  Dim obottom = o.top + o.height;
  if (o.left<left)
    left = o.left;
  if (o.top<top)
    top = o.top;
  if (oright>right)
    right = oright;
  if (obottom>bottom)
    bottom = obottom;
  width = right-left;
  height = bottom-top;
  return *this;
}

bool Rect::intersects(Rect const &o) const {
  return o.right()>left && right()>o.left
    && o.bottom()>top && bottom()>o.top;
}

bool Rect::contains(Rect const &o) const {
  return o.left>=left && o.top>=top
    && o.right()<=right() && o.bottom()<=bottom() ;
}

bool Rect::contains(Point const &p) const {
  return p.x>=left && p.y>=top
    && p.x<=right() && p.y<=bottom();
}

Rect Rect::translated(Point const &dxy) const {
  Rect r = *this;
  r.left += dxy.x;
  r.top += dxy.y;
  return r;
}

