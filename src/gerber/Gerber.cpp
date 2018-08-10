// Gerber.cpp

#include "Gerber.h"

namespace Gerber {
  QString real(Dim d) {
    return QString::number(d.toMM());
  }
  QString coord(Dim d) {
    constexpr int FACTOR = PERUM / Dim::PerUM;
    return QString::number(d.raw()*FACTOR);
  }
  QString point(Point p) {
    return "X" + coord(p.x) + "Y" + coord(p.y);
  }
  QString layerInfix(Layer l) {
    switch (l) {
    case Gerber::Layer::BoardOutline: return "Profile";
    case Gerber::Layer::ThroughHoles: return "Drill";
    case Gerber::Layer::BottomCopper: return "Bottom";
    case Gerber::Layer::BottomSolderMask: return "BottomMask";
    case Gerber::Layer::TopCopper: return "Top";
    case Gerber::Layer::TopSolderMask: return "TopMask";
    case Gerber::Layer::TopSilk: return "Silk";
    default: return "";
    }
  }
};

  
