// SimpleFont.cpp

#include "SimpleFont.h"

#include "font.cpp"

SimpleFont::SimpleFont() {
  font const &ft = simpleFont();
  for (char c: ft.keys()) {
    QVector<QPolygonF> w;
    for (polyline v: ft[c]) {
      QPolygonF pp;
      while (!v.isEmpty()) {
	int x = v.takeFirst();
	int y = v.takeFirst();
	pp << QPointF(x,y);
      }
      w << pp;
    }
    chars[c] = w;
  }
}

QVector<QPolygonF> const &SimpleFont::character(char c) const {
  static QVector<QPolygonF> nul;
  auto it(chars.find(c));
  return it == chars.end() ? nul : *it;
}

SimpleFont const &SimpleFont::instance() {
  static SimpleFont font;
  return font;
}

double SimpleFont::baseSize() const {
  return 20;
}

double SimpleFont::dx() const {
  return 18;
}

double SimpleFont::width(QString s) const {
  return dx()*s.size();
}

double SimpleFont::ascent() const {
  return 20;
}

double SimpleFont::descent() const {
  return 9;
}


