// Layer.cpp

#include "Layer.h"

QColor const &layerColor(Layer l, bool sel) {
  auto makesel = [](QColor c) {
    c = c.toHsl();
    int l = 3 * c.lightness() / 2;
    c.setHsl(c.hslHue(), c.hslSaturation()/2, l>255 ? 255 : l);
    return c.toRgb();
  };
  static QColor silk(240, 240, 0);
  static QColor top(255, 0, 0);
  static QColor bottom(0, 180, 0);
  static QColor invalid(0, 0, 0);
  static QColor selsilk = makesel(silk);
  static QColor seltop = makesel(top);
  static QColor selbottom = makesel(makesel(bottom));
  switch (l) {
  case Layer::Invalid: return invalid;
  case Layer::Silk: return sel ? selsilk : silk;
  case Layer::Top: return sel ? seltop : top;
  case Layer::Bottom: return sel ? selbottom: bottom;
  }
  return invalid;
}

QDebug operator<<(QDebug d, Layer const &l) {
  switch (l) {
  case Layer::Silk: d << "Silk"; break;
  case Layer::Top: d << "Top"; break;
  case Layer::Bottom: d << "Bottom"; break;
  default: d << "Invalid"; break;
  }
  return d;
}
