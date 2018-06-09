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
private:
  QMap<char, QVector<QPolygonF> > chars;
};

#endif
