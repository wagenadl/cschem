// FabSettings.h

#ifndef FABSETTINGS_H

#define FABSETTINGS_H

#include "ScalingDim.h"
#include <QXmlStreamReader>
#include <QDebug>

class Settings {
public:
  ScalingDim trace_clearance;
  ScalingDim pad_clearance;
  ScalingDim thermal_width;
  ScalingDim mask_margin;
  Dim hole_min, hole_max, ring_min;
public:
  Settings();
};

QDebug operator<<(QDebug, Board const &);
QXmlStreamWriter &operator<<(QXmlStreamWriter &, Board const &);
QXmlStreamReader &operator>>(QXmlStreamReader &, Board &);

#endif
