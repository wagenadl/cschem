// FabSettings.h

#ifndef FABSETTINGS_H

#define FABSETTINGS_H

#include "ScalingDim.h"
#include <QXmlStreamReader>
#include <QDebug>

class FabSettings {
public:
  ScalingDim trace_clearance;
  ScalingDim pad_clearance;
  ScalingDim thermal_width;
  ScalingDim mask_margin;
  Dim hole_min, hole_max, ring_min;
public:
  FabSettings();
  void save() const;
  void resetToSaved();
  void restoreFactoryDefaults(); 
};

QDebug operator<<(QDebug, FabSettings const &);
QXmlStreamWriter &operator<<(QXmlStreamWriter &, FabSettings const &);
QXmlStreamReader &operator>>(QXmlStreamReader &, FabSettings &);

#endif
