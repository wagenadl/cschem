// Font.cpp

#include "Font.h"
#include "Gerber.h"

namespace Gerber {
  Font::Font(Apertures *ap0, FontSpec spec):
    ap(ap0), spec(spec), sf(&SimpleFont::instance()) {
    lw  = sf->scaleFactor(spec.fs) * sf->lineWidth();
    ap0->ensure(Circ(lw));
  }

  void Font::writeChar(QTextStream &, Point, char c) const {
    if (c<=32 || c>126)
      return; // nothing to draw (note: space doesn't need to be drawn)

    QVector<Polyline> strokes = sf->character(c);
    for (Polyline const &pp: strokes) {
      int K = pp.size();
      if (K<2)
	continue;
      
      //	output << point(Point::fromMils(pp[0])) << "D02*\n";
      //	for (int k=1; k<K; k++)
      //	  output << point(Point::fromMils(pp[k])) << "D01*\n";
    }
  }

  void Font::writeText(QTextStream &output, Text const &txt) const {
    ap->select(Circ(lw));
    output << "G01*\n"; // select linear interpolation
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
    
