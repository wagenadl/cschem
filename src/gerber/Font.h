// Font.h

#ifndef GERBERFONT_H

#define GERBERFONT_H

#include "Apertures.h"
#include "FontSpec.h"
#include "data/SimpleFont.h"
#include "data/Point.h"
#include "data/Text.h"

namespace Gerber {
  class Font {
  public:
    Font(Apertures *ap, FontSpec);
    void writeText(QTextStream &output, Text const &) const;
  private:
    void writeChar(QTextStream &output, Point, char) const;
  private:
    Apertures const *ap;
    FontSpec spec;
    double scl;
    Dim lw;
    SimpleFont const *sf;
  };
};

#endif
