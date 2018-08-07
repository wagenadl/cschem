// SimpleFont.h

#ifndef SIMPLEFONT_H

#define SIMPLEFONT_H

#include <QVector>
#include <QPolygonF>
#include <QMap>

class SimpleFont {
public:
  SimpleFont();
  QVector<QPolygonF> const &character(char) const;
  static SimpleFont const &instance();
  double baseSize() const; // design size in mils
  double baseLinewidth() const; // design linewidth in mils
  double dx() const; // design horizontal step between characters (mils)
  double width(QString) const;
  double ascent() const; // design ascent (mils)
  double descent() const; // design descent (mils)
  
private:
  QMap<char, QVector<QPolygonF> > chars;
};

#endif
