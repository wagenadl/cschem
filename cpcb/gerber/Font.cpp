// Font.cpp

#include "Font.h"
#include "Gerber.h"

namespace Gerber {
  Font::Font(Apertures *ap0, FontSpec spec):
    ap(ap0), spec(spec), sf(&SimpleFont::instance()) {
    scl = sf->scaleFactor(spec.fs);
    lw  = scl * sf->lineWidth();
    ap0->ensure(Circ(lw));
  }

  void Font::writeChar(QTextStream &output, Point p0, char c) const {
    if (c<=32 || c>126)
      return; // nothing to draw (note: space doesn't need to be drawn)

    auto map = [&](Point p) {
      // map the font point p to external coordinates
      int sgn = spec.flip ? -1 : 1;
      Dim x = scl*sgn*p.x;
      Dim y = -scl*p.y;
      switch (spec.rot & 3) {
      case 0: return point(p0 + Point(x, y));
      case 1: return point(p0 + Point(-y, x));
      case 2: return point(p0 + Point(-x, -y));
      case 3: return point(p0 + Point(y, -x));
      }
      return point(Point()); // not executed
    };
    
    QVector<Polyline> strokes = sf->character(c);
    for (Polyline const &pp: strokes) {
      int K = pp.size();
      if (K==1) {
	output << map(pp[0]) << "D03*\n";
      } else if (K>=2) {
	output << map(pp[0]) << "D02*\n";
	for (int k=1; k<K; k++)
	  output << map(pp[k]) << "D01*\n";
      }
    }
  }

  void Font::writeText(QTextStream &output, Text const &txt) const {
    output << "G01*\n"; // select linear interpolation
    output << ap->select(Circ(lw));
    double scl = sf->scaleFactor(txt.fontsize);
    Dim dx = sf->dx() * scl;
    Point dxy;
    int sgn = txt.orient.flip ? -1 : 1;
    switch (txt.orient.rot & 3) {
    case 0: dxy = Point(sgn*dx, Dim()); break;
    case 1: dxy = Point(Dim(), sgn*dx); break;
    case 2: dxy = Point(-sgn*dx, Dim()); break;
    case 3: dxy = Point(Dim(), -sgn*dx); break;
    }
    Point p = txt.p;
    for (int k=0; k<txt.text.size(); k++) {
      writeChar(output, p, txt.text[k].unicode());
      p += dxy;
    }
  }
};
    
