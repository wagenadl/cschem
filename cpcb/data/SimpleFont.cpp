// SimpleFont.cpp

#include "SimpleFont.h"

#include "font.h"

SimpleFont::SimpleFont() {
  font const &ft = simpleFont();
  Dim dy = baselineShift();
  for (int c: ft.keys()) {
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
  chars[0x2212] = chars[0x2d]; // minus -> ascii
  chars[0x2013] = chars[0x2d]; // en-dash -> ascii
}

QVector<Polyline> const &SimpleFont::character(int c) const {
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
  return Dim::fromMils(3);
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

/*
QVector<QPolygonF> const &SimpleFont::sizedCharacter(char c, Dim fs) const {
  if (sizedchars[fs].contains(c))
    return sizedchars[fs][c];
  double scl = scaleFactor(fs);
  QVector<QPolygonF> glyphs;
  QVector<Polyline> const &src = character(c);
  for (Polyline const  &pp0: src) {
    QPolygonF pp;
    for (Point const &p0: pp0)
      pp << p0.toMils()*scl;
    glyphs << pp;
  }
  sizedchars[fs][c] = glyphs;
  auto it = sizedchars[fs].find(c);
  return *it;
}
*/
