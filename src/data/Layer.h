// Layer.h

#ifndef LAYER_H

#define LAYER_H

#include <QDebug>

enum class Layer {
  Invalid,
  Silk,
  Top,
  Bottom,
};

QDebug operator<<(QDebug, Layer const &);


#endif
