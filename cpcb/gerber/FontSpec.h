// FontSpec.h

#ifndef FONTSPEC_H

#define FONTSPEC_H

#include "data/Dim.h"
#include "data/FreeRotation.h"

namespace Gerber {
  struct FontSpec {
    Dim fs;
    FreeRotation rota;
    bool flip;    
    FontSpec(Dim fs, FreeRotation rot, bool flip):
      fs(fs), rota(rot), flip(flip) {}
    bool operator<(FontSpec const &o) const {
      if (fs == o.fs) {
	if (int(rota) == int(o.rota))
	  return flip < o.flip;
	return int(rota) < int(o.rota);
      }
      return fs < o.fs;
    }
  };

  inline QDebug operator<<(QDebug d, FontSpec const &fs) {
    d << "fontspec" << fs.fs.toMils() << fs.rota << fs.flip;
    return d;
  }
};

#endif
