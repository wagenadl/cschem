// ScalingDim.h

#ifndef SCALINGDIM_H

#define SCALINGDIM_H

#include "Dim.h"
#include <QXmlStreamReader>
#include <QDebug>

class ScalingDim {
public:
  Dim min;
  Dim max;
  float perc;
  bool scale;
public:
  ScalingDim(Dim fixed);
  ScalingDim(Dim min, float perc, Dim max);
  Dim apply(Dim x) const;
};

QDebug operator<<(QDebug, ScalingDim const &);
QXmlStreamWriter &operator<<(QXmlStreamWriter &, ScalingDim const &);
QXmlStreamReader &operator>>(QXmlStreamReader &, ScalingDim &);

#endif
