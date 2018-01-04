// Style.h

#ifndef STYLE_H

#define STYLE_H

#include <QColor>

class Style {
public:
  static QColor connectionColor();
  static QColor danglingColor();
  static QColor elementHoverColor();
  static QColor selectedElementHoverColor();
  static QColor connectionHoverColor();
  static QColor pinHighlightColor();
  static QColor selectionColor();
  static double connectionHoverWidthFactor();
  static double connectionDraftWidthFactor();
  static double selectionRectRadius();
};

#endif
