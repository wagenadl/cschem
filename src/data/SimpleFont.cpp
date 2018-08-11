// SimpleFont.cpp

#include "SimpleFont.h"

#include "font.cpp"

SimpleFont::SimpleFont() {
  font const &ft = simpleFont();
  Dim dy = baselineShift();
  for (char c: ft.keys()) {
    QVector<Polyline> w;
    for (polyline v: ft[c]) {
      Polyline pp;
      while (!v.isEmpty()) {
	int x = v.takeFirst();
	int y = v.takeFirst();
	pp << Point(Dim::fromMils(x), Dim::fromMils(y) - dy);
      }
      w << pp;
    }
    chars[c] = w;
  }
}

QVector<Polyline> const &SimpleFont::character(char c) const {
  static QVector<Polyline> nul;
  auto it(chars.find(c));
  return it == chars.end() ? nul : *it;
}

SimpleFont const &SimpleFont::instance() {
  static SimpleFont font;
  return font;
}

double SimpleFont::scaleFactor(Dim fs) const {
  return fs.toMils() / fontSize().toMils();
}

Dim SimpleFont::fontSize() const {
  return Dim::fromMils(20);
}

Dim SimpleFont::lineWidth() const {
  return Dim::fromMils(4);
}

Dim SimpleFont::dx() const {
  return Dim::fromMils(18);
}

Dim SimpleFont::width(QString s) const {
  return dx()*s.size();
}

Dim SimpleFont::baselineShift() const {
  return Dim::fromMils(20/2);
}

Dim SimpleFont::ascent() const {
  return Dim::fromMils(20) - baselineShift();
}

Dim SimpleFont::descent() const {
  return Dim::fromMils(9) + baselineShift();
}
