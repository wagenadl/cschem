// Layer.cpp

#include "Layer.h"
#include <QMap>
#include <QDebug>

static QMap<Layer, QString> const &layerAbbrevMap() {
  static QMap<Layer, QString> map{
    { Layer::Schematic, "" },
    { Layer::TopSilk, "TS" },
    { Layer::BottomSilk, "BS" },
    { Layer::Top, "T" },
    { Layer::Bottom, "B" },
    { Layer::Inner1, "I1" },
    { Layer::Inner2, "I2" },
    { Layer::Inner3, "I3" },
    { Layer::Inner4, "I4" }
  };
  return map;
}

static QMap<QString, Layer> const &layerAbbrevRevMap() {
  static QMap<QString, Layer> rev;
  if (rev.isEmpty()) {
    auto const &map = layerAbbrevMap();
    for (Layer l: map.keys())
      rev[map[l]] = l;
  }
  return rev;
}

QString layerToAbbreviation(Layer layer) {
  auto const &map = layerAbbrevMap();
  if (map.contains(layer)) {
    return map[layer];
  } else {
    qDebug() << "UNKNOWN LAYER";
    return "";
  }
}

Layer layerFromAbbreviation(QString abbrev) {
  auto const &rev = layerAbbrevRevMap();
  if (rev.contains(abbrev)) {
    return rev[abbrev];
  } else {
    qDebug() << "UNKNOWN LAYER ABBREVIATION";
    return Layer::Schematic;
  }
}
