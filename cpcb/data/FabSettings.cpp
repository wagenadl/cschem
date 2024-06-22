// FabSettings.cpp

#include "FabSettings.h"
#include <QSettings>

FabSettings::FabSettings() {
  resetToSaved();
}

void FabSettings::resetToSaved() {
  QSettings ss;
  if (!ss.contains("trace_clearance_min")) {
    restoreFactoryDefaults();
    return;
  }
  trace_clearance.fromSettings("trace_clearance");
  pad_clearance.fromSettings("pad_clearance");
  thermal_width.fromSettings("thermal_width");
  mask_margin.fromSettings("mask_margin");
  hole_min = Dim::fromUM(ss.value("hole_min", 300).toInt());
  hole_max = Dim::fromUM(ss.value("hole_max", 3000).toInt());
  ring_min = Dim::fromUM(ss.value("ring_min", 300).toInt());
}

void FabSettings::restoreFactoryDefaults() {
  trace_clearance = ScalingDim(Dim::fromMils(15));
  pad_clearance = ScalingDim(Dim::fromMils(15));
  thermal_width = ScalingDim(Dim::fromMils(12));
  mask_margin = ScalingDim(Dim::fromMils(10));
  hole_min = Dim::fromUM(300);
  hole_max = Dim::fromMM(7.0);
  ring_min = Dim::fromUM(300);
}

void FabSettings::save() const {
  QSettings ss;
  trace_clearance.toSettings("trace_clearance");
  pad_clearance.toSettings("pad_settings");
  thermal_width.toSettings("thermal_width");
  mask_margin.toSettings("mask_margin");
  ss.setValue("hole_min", hole_min.toUM());
  ss.setValue("hole_max", hole_max.toUM());
  ss.setValue("ring_min", ring_min.toUM());
}
