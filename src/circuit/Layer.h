// Layer.h

#ifndef LAYER_H

#define LAYER_H

#include <QString>

enum class Layer {
  Schematic,
  TopSilk,
  BottomSilk,
  Top,
  Bottom,
  Inner1,
  Inner2,
  Inner3,
  Inner4
}; // only relevant for board

QString layerToAbbreviation(Layer);
Layer layerFromAbbreviation(QString);

#endif
