// Style.h

#ifndef STYLE_H

#define STYLE_H

#include <QColor>

class Style {
public:
  static QColor connectionColor();
  static QColor danglingColor();
  static QColor elementHoverColor();
  static QColor connectionHoverColor();
  static QColor pinHighlightColor();
  static double connectionHoverWidthFactor();
  static double connectionDraftWidthFactor();
};

#endif
