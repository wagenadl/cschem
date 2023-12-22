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
    case Gerber::Layer::BoardOutline: return "BoardOutline";
    case Gerber::Layer::ThroughHoles: return "drill_PTH";
    case Gerber::Layer::BottomCopper: return "BottomLayer";
    case Gerber::Layer::BottomSolderMask: return "BottomSolderMaskLayer";
    case Gerber::Layer::BottomSilk: return "BottomSilkLayer";
    case Gerber::Layer::BottomPasteMask: return "BottomPasteMaskLayer";
    case Gerber::Layer::TopCopper: return "TopLayer";
    case Gerber::Layer::TopSolderMask: return "TopSolderMaskLayer";
    case Gerber::Layer::TopSilk: return "TopSilkLayer";
    case Gerber::Layer::TopPasteMask: return "TopPasteMaskLayer";
    case Gerber::Layer::NonplatedHoles: return "drill_NPTH";
    default: return "";
    }
  }
  QString layerSuffix(Layer l) {
    switch (l) {
    case Gerber::Layer::BoardOutline: return "GKO";
    case Gerber::Layer::ThroughHoles: return "DRL";
    case Gerber::Layer::NonplatedHoles: return "DRL";
    case Gerber::Layer::BottomCopper: return "GBL";
    case Gerber::Layer::BottomSolderMask: return "GBS";
    case Gerber::Layer::BottomPasteMask: return "GBP";
    case Gerber::Layer::BottomSilk: return "GBO";
    case Gerber::Layer::TopCopper: return "GTL";
    case Gerber::Layer::TopSolderMask: return "GTS";
    case Gerber::Layer::TopPasteMask: return "GTP";
    case Gerber::Layer::TopSilk: return "GTO";
    default: return "";
    }
  }
};

 
