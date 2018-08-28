// Layer.h

#ifndef LAYER_H

#define LAYER_H

#include <QDebug>
#include <QColor>

enum class Layer {
  Invalid = 0,
  Silk,
  Top,
  Bottom,
};

QColor const &layerColor(Layer, bool selected=false);
QList<Layer> const &layers();

QDebug operator<<(QDebug, Layer const &);


#endif
