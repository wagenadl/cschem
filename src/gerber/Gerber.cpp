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
    case Gerber::Layer::BoardOutline: return "0-Profile";
    case Gerber::Layer::ThroughHoles: return "1-Drill";
    case Gerber::Layer::BottomCopper: return "5-Bottom";
    case Gerber::Layer::BottomSolderMask: return "6-BottomMask";
    case Gerber::Layer::TopCopper: return "3-Top";
    case Gerber::Layer::TopSolderMask: return "4-TopMask";
    case Gerber::Layer::TopSilk: return "2-Silk";
    default: return "";
    }
  }
};

  
