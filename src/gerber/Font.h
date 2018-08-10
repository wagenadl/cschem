// Font.h

#ifndef GERBERFONT_H

#define GERBERFONT_H

#include "Apertures.h"
#include "data/SimpleFont.h"

namespace Gerber {
  class Font {
  public:
    Font(Apertures &ap);
    void writeFont(QTextStream &output);
    void ensure(char);
    void ensure(QString);
    void writeChar(QTextStream &output, char);
    void writeString(QTextStream &output, QString);
  private:
    QSet<char> got;
    Apertures const &ap;
    SimpleFont const &sf;
  };
};

#endif
