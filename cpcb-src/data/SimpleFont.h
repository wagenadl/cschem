// SimpleFont.h

#ifndef SIMPLEFONT_H

#define SIMPLEFONT_H

#include "Polyline.h"
#include <QVector>
#include <QMap>
#include <QPolygonF>

class SimpleFont {
public:
  SimpleFont();
  QVector<Polyline> const &character(char) const;
  // QVector<QPolygonF> const &sizedCharacter(char, Dim fs) const;
  static SimpleFont const &instance();
  Dim fontSize() const; // nominal font size
  Dim lineWidth() const;
  Dim dx() const; // horizontal step between characters
  Dim width(QString) const;
  Dim ascent() const; // ascent - note that our baseline is the middle of "X".
  Dim descent() const; // descent (as a positive number)
  Dim baselineShift() const; // (positive) distance between nominal baseline and
                             // bottom of "X".
  double scaleFactor(Dim altFontSize) const;
private:
  QMap<char, QVector<Polyline>> chars;
  // mutable QMap<Dim, QMap<char, QVector<QPolygonF>>> sizedchars;
};

#endif
