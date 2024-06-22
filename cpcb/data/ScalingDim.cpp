// ScalingDim.cpp

#include "ScalingDim.h"
#include <QSettings>

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
  if (sd.scale)
    d << QString("ScalingDim(%1,%2,%3)")
      .arg(sd.min.toMM())
      .arg(sd.perc)
      .arg(sd.max.toMM());
  else
    d << QString("ScalingDim(%1)")
      .arg(sd.min.toMM());
  return d;
}

QXmlStreamWriter &operator<<(QXmlStreamWriter &s, ScalingDim const &sd) {
  s.writeAttribute("min", sd.min.toString());
  if (sd.scale) {
    s.writeAttribute("max", sd.max.toString());
    s.writeAttribute("perc", QString::number(sd.perc));
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

void ScalingDim::fromSettings(QString name) {
  QSettings ss;
  min = Dim::fromUM(ss.value(name + "_min", 0).toInt());
  scale = ss.value(name + "_scale", false).toBool();
  max = Dim::fromUM(ss.value(name + "_max", 1000).toInt());
  perc = ss.value(name + "_perc", 100).toDouble();
}

void ScalingDim::toSettings(QString name) const {
  QSettings ss;
  ss.setValue(name + "_min", min.toUM());
  ss.setValue(name + "_scale", scale);
  ss.setValue(name + "_max", max.toUM());
  ss.setValue(name + "_perc", perc);
}
