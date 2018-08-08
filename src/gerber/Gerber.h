// Gerber.h

#ifndef GERBER_H

#define GERBER_H

#include "data/Dim.h"
#include <QString>

namespace Gerber {
  enum class Layer {
    BoardOutline,
    ThroughHoles,
    BottomCopper,
    BottomSolderMask,
    TopCopper,
    TopSolderMask,
    TopSilk
  };
  enum class Polarity {
    Positive,
    Negative
  };
  QString coord(Dim);
  QString real(Dim);
  QString layerInfix(Layer);
};

#endif
