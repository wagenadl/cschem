// FontSpec.h

#ifndef FONTSPEC_H

#define FONTSPEC_H

#include "data/Dim.h"

namespace Gerber {
  struct FontSpec {
    Dim fs;
    int rot;
    bool flip;    
    FontSpec(Dim fs, int rot, bool flip):
      fs(fs), rot(rot&3), flip(flip) {}
    bool operator<(FontSpec const &o) const {
      if (fs == o.fs) {
	if (rot == o.rot)
	  return flip < o.flip;
	return rot < o.rot;
      }
      return fs < o.fs;
    }
  };
};

#endif
