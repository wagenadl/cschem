// ScalingDim.cpp

#include "ScalingDim.h"

ScalingDim::ScalingDim(Dim fixed):
  min(fixed), max(fixed), perc(100), scale(false) {
}

ScalingDim::ScalingDim(Dim min, float perc, Dim max):
  min(min), max(max), perc(perc), scale(true) {
}

Dim ScalingDim::apply(Dim x) const {
  if (scale) {
    Dim y = (perc/100)*x;
    if (y<min)
      return min;
    else if (y>max)
      return max;
    else
      return y;
  } else {
    return min;
  }
};

QDebug operator<<(QDebug d, ScalingDim const &sd) {
  if (scale)
    d << QString("ScalingDim(%1,%2,%3)")
      .arg(min.toMM())
      .arg(perc)
      .arg(max.toMM());
  else
    d << QString("ScalingDim(%1)")
      .arg(min.toMM());
  return d;
}

QXmlStreamWriter &operator<<(QXmlStreamWriter &s, ScalingDim const &sd) {
  s.writeAttribute("min", min.toString());
  if (scale) {
    s.writeAttribute("max", max.toString());
    s.writeAttribute("perc", QString::number(perc));
  }
  return s;
}

QXmlStreamReader &operator>>(QXmlStreamReader &s, ScalingDim &sd) {
  auto a = s.attributes();
  sd.min = Dim::fromString(a.value("min").toString());
  if (a.hasAttribute("max")) {
    sd.max = Dim::fromString(a.value("max").toString());
    sd.perc = a.value("perc").toDouble();
    sd.scale = true;
  } else {
    sd.max = sd.min;
    sd.perc = 100;
    sd.scale = false;
  }
  return s;
}
