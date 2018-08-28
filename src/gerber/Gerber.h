// Gerber.h

#ifndef GERBER_H

#define GERBER_H

#include "data/Dim.h"
#include "data/Point.h"
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
  constexpr int PERUM = 100; // assuming 5 digits past mm
  QString coord(Dim);
  QString point(Point); // include "X" and "Y" prefixes
  QString real(Dim); // in millimeters
  QString layerInfix(Layer);
  QString layerSuffix(Layer);
};

#endif
