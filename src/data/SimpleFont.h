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
  double baseSize() const;
  double dx() const;
  double width(QString) const;
  double ascent() const;
  double descent() const;
  
private:
  QMap<char, QVector<QPolygonF> > chars;
};

#endif
