// Style.h

#ifndef STYLE_H

#define STYLE_H

#include <QColor>
#include <QFont>

class Style {
public:
  static QColor danglingColor();
  static QColor hoverColor();
  static QColor selectedElementHoverColor();
  static QColor pinHighlightColor();
  static QColor selectionBackgroundColor();
  static QColor faintColor();
  static QColor faintHoverColor();
  static QColor textColor();
  static QColor layerColor();
  static double connectionHoverWidthFactor();
  static double connectionDraftWidthFactor();
  static double selectionRectRadius();
  static QString programName();
  static QString versionName();
  static QFont annotationFont();
};

#endif
