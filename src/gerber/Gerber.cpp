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
    case Gerber::Layer::BottomCopper: return "6-Bottom";
    case Gerber::Layer::BottomSolderMask: return "7-BottomMask";
    case Gerber::Layer::TopCopper: return "4-Top";
    case Gerber::Layer::TopSolderMask: return "5-TopMask";
    case Gerber::Layer::TopSilk: return "2-Silk";
    case Gerber::Layer::TopPasteMask: return "3-TopPaste";
    default: return "";
    }
  }
  QString layerSuffix(Layer l) {
    switch (l) {
    case Gerber::Layer::BoardOutline: return "GKO";
    case Gerber::Layer::ThroughHoles: return "DRI";
    case Gerber::Layer::BottomCopper: return "GBL";
    case Gerber::Layer::BottomSolderMask: return "GBS";
    case Gerber::Layer::TopCopper: return "GTL";
    case Gerber::Layer::TopSolderMask: return "GTS";
    case Gerber::Layer::TopPasteMask: return "GTP";
    case Gerber::Layer::TopSilk: return "GTO";
    default: return "";
    }
  }
};

  
