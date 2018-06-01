// Layer.cpp

#include "Layer.h"

QDebug operator<<(QDebug d, Layer const &l) {
  switch (l) {
  case Layer::Silk: d << "Silk"; break;
  case Layer::Top: d << "Top"; break;
  case Layer::Bottom: d << "Bottom"; break;
  default: d << "Invalid"; break;
  }
  return d;
}
