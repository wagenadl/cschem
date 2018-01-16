// Style.h

#ifndef STYLE_H

#define STYLE_H

#include <QColor>
#include <QFont>
#include "circuit/Layer.h"

class Style {
public:
  static QColor danglingColor();
  static QColor hoverColor(Layer l=Layer::Schematic);
  static QColor selectedElementHoverColor(Layer l=Layer::Schematic);
  static QColor pinHighlightColor();
  static QColor selectionBackgroundColor();
  static QColor faintColor();
  static QColor textColor(Layer l=Layer::Schematic);
  static QColor layerColor(Layer l=Layer::Schematic);
  static double connectionHoverWidthFactor();
  static double connectionDraftWidthFactor();
  static double selectionRectRadius();
  static QString programName();
  static QString versionName();
  static QFont annotationFont();
};

#endif
