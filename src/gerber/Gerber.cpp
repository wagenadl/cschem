// Gerber.cpp

#include "Gerber.h"

namespace Gerber {
  QString coord(Dim d) {
    return QString::number(d.toMM());
  }
  QString real(Dim d) {
    constexpr int PERUM = 100;
    constexpr int FACTOR = PERUM / Dim::PerUM;
    return QString::number(d.raw()*FACTOR);
  }
  QString layerInfix(Layer l) {
    switch (l) {
    case BoardOutline: return "Profile";
    case ThroughHoles: return "Drill";
    case BottomCopper: return "Bottom";
    case BottomSolderMask: return "BottomMask";
    case TopCopper: return "Top";
    case TopSolderMask: return "TopMask";
    case TopSilk: return "Silk";
    default: return "";
    }
  }
};

  
