// Font.h

#ifndef GERBERFONT_H

#define GERBERFONT_H

#include "Aperture.h"

namespace Gerber {
  class Font {
  public:
    Font(Aperture &ap);
    void writeFont(QTextStream &output);
    void ensure(char);
    void ensure(QString);
    void writeChar(QTextStream &output, char);
    void writeString(QTextStream &output, QString);
  private:
    QSet<char> got;
    Aperture const &ap;
    class SimpleFont const &sf;
  };
};

#endif
