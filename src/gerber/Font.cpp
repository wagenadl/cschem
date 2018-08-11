// Font.cpp

#include "Font.h"
#include "Gerber.h"

namespace Gerber {
  
  Font::Font(Apertures &ap0):
    ap(ap0), sf(SimpleFont::instance()) {
    ap0.ensure(Circ(Dim::fromMils(sf.baseLinewidth())));
  }

  void Font::ensure(char c) {
    got << c;
  }

  void Font::ensure(QString s) {
    int N = s.size();
    for (int n=0; n<N; n++) {
      int c = s[n].unicode();
      if (c>=32 && c<=126)
	ensure(char(c));
    }
  }

  void Font::writeFont(QTextStream &output) const {
    for (char c: got) {
      int idx = c; 
      output << "G04 character " << idx << " " << c << " *\n";
      output << "%ABD" << int(c) << "*%\n";
      output << "G01*\n"; // select linear interpolation
      ap.select(Circ(Dim::fromMils(sf.baseLinewidth())));
      // now draw the character
      QVector<QPolygonF> strokes = sf.character(c);
      for (QPolygonF const &pp: strokes) {
	int K = pp.size();
	if (K<2)
	  continue;
	output << point(Point::fromMils(pp[0])) << "D02*\n";
	for (int k=1; k<K; k++)
	  output << point(Point::fromMils(pp[k])) << "D01*\n";
      }
      output << "%AB*%\n";
    }
  }

  void Font::writeChar(QTextStream &output, Point p, char c) const {
    if (c<=32 || c>126)
      return; // nothing to draw (note: space doesn't need to be drawn)
    output << "D" << int(c) << "*\n";
    output << point(p) << "D03*\n";
  }

  void Font::writeText(QTextStream &output, Text const &txt) const {
    SimpleFont const &sf(SimpleFont::instance());
    switch (txt.orient.rot & 3) {
    case 0: output << "%LR0*%\n"; break;
    case 1: output << "%LR270*%\n"; break;
    case 2: output << "%LR180*%\n"; break;
    case 3: output << "%LR90*%\n"; break;
    }
    if (txt.orient.flip)
      output << "%LMX*%\n";
    else
      output << "%LMN*%\n";
    double scl = txt.fontsize.toMils() / sf.baseSize();
    Dim dx = Dim::fromMils(sf.dx() * scl);
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
    
