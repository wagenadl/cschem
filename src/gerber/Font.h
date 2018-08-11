// Font.h

#ifndef GERBERFONT_H

#define GERBERFONT_H

#include "Apertures.h"
#include "data/SimpleFont.h"
#include "data/Point.h"
#include "data/Text.h"

namespace Gerber {
  class Font {
  public:
    constexpr static int FirstSafeAperture = 200;
    /* This is an aperture number above which none are used by the font. */
  public:
    Font(Apertures &ap);
    void writeFont(QTextStream &output) const;
    void ensure(char);
    void ensure(QString);
    void writeChar(QTextStream &output, Point, char) const;
    void writeText(QTextStream &output, Text const &) const;
  private:
    QSet<char> got;
    Apertures const &ap;
    SimpleFont const &sf;
  };
};

#endif
