// Style.h

#ifndef STYLE_H

#define STYLE_H

#include <QColor>
#include <QFont>

class Style {
public:
  static QColor connectionColor();
  static QColor danglingColor();
  static QColor elementHoverColor();
  static QColor selectedElementHoverColor();
  static QColor connectionHoverColor();
  static QColor pinHighlightColor();
  // static QColor magnetHighlightColor();
  static QColor selectionColor();
  static QColor faintColor();
  static QColor textColor();
  static double connectionHoverWidthFactor();
  static double connectionDraftWidthFactor();
  static double selectionRectRadius();
  static QString programName();
  static QString versionName();
  static QFont annotationFont();
};

#endif
