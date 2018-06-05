// Layer.h

#ifndef LAYER_H

#define LAYER_H

#include <QDebug>
#include <QColor>

enum class Layer {
  Invalid,
  Silk,
  Top,
  Bottom,
};

QColor const &layerColor(Layer, bool selected=false);

QDebug operator<<(QDebug, Layer const &);


#endif
