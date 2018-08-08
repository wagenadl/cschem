// Font.cpp

#include "Font.h"
#include "Gerber.h"
#include "data/SimpleFont.h"

namespace Gerber {
  
  Font::Font(Aperture &ap0):
    ap(ap0), sf(SimpleFont::instance()) {
    ap0.ensure(Circ(sf.baseLinewidth()));
  }

  void Font::ensure(char c) {
    got << c;
  }

  void Font::ensure(QString s) {
    int N = s.size();
    for (int n=0; n<N; n++) {
      int c = s[n];
      if (c>=32 && c<=126)
	ensure(char(c));
    }
  }

  void Font::writeFont(QTextStream &output) {
    for (char c: got) {
      int idx = c; 
      output << "G04 character " << QString::number(idx) << " *\n";
      output << "%ABD" << c << "*%\n";
      output << "G01*\n"; // select linear interpolation
      ap.select(Circ(sf.baseLineWidth()));
      // now draw the character
      QVector<QPolygonF> strokes = sf.character(c);
      for (QPolygonF const &pp: strokes) {
	int K = pp.size();
	if (K<2)
	  continue;
	output << "X" << real(Dim::fromMils(pp[0].x))
	       << "Y" << real(Dim::fromMils(pp[0].y))
	       << "D02*\n";
	for (int k=1; k<K; k++)
	  output << "X" << real(Dim::fromMils(pp[k].x))
		 << "Y" << real(Dim::fromMils(pp[k].y))
		 << "D01*\n";
      }
      output << "%AB*%\n";
    }
  }

  void Font::writeChar(QTextStream &output, char c) {
  }

  void Font::writeString(QTextStream &output, QString s) {
  }
};
    
