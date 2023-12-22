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
  static QColor bsilk(48, 255, 255);
  static QColor top(255, 0, 0);
  static QColor bottom(0, 180, 0);
  static QColor invalid(0, 0, 0);
  static QColor selsilk = makesel(makesel(silk));
  static QColor selbsilk = makesel(makesel(bsilk));
  static QColor seltop = makesel(top);
  static QColor selbottom = makesel(makesel(bottom));
  static QColor panel(150, 100, 255);
  static QColor selpanel = makesel(panel);
  switch (l) {
  case Layer::Invalid: return invalid;
  case Layer::Silk: return sel ? selsilk : silk;
  case Layer::BSilk: return sel ? selbsilk : bsilk;
  case Layer::Top: return sel ? seltop : top;
  case Layer::Bottom: return sel ? selbottom: bottom;
  case Layer::Panel: return sel ? selpanel: panel;
  }
  return invalid;
}

QList<Layer> const &layers() {
  static QList<Layer> lays{Layer::Panel, Layer::Silk, Layer::Top, Layer::BSilk, Layer::Bottom};
  return lays;
}

QDebug operator<<(QDebug d, Layer const &l) {
  switch (l) {
  case Layer::Silk: d << "Silk"; break;
  case Layer::BSilk: d << "BSilk"; break;
  case Layer::Top: d << "Top"; break;
  case Layer::Bottom: d << "Bottom"; break;
  case Layer::Panel: d << "Panel"; break;
  default: d << "Invalid"; break;
  }
  return d;
}

