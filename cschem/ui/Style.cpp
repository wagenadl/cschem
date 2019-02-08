// Style.cpp

#include "Style.h"
#include "Version.h"

QColor Style::faintColor() {
  return QColor(180, 180, 180);
}

QColor Style::layerColor() {
  return QColor(0, 0, 0);
}

QColor Style::textColor() {
  return QColor(0, 0, 0);
}

QColor Style::danglingColor() {
  return QColor(255, 0, 0);
}

QColor Style::hoverColor() {
  return QColor(64, 192, 255);
}

QColor Style::selectedElementHoverColor() {
  return QColor(0, 64, 255);
}

QColor Style::pinHighlightColor() {
  return QColor(0, 255, 128);
}

// QColor Style::magnetHighlightColor() {
//   return QColor(0, 255, 0);
// }

QColor Style::selectionBackgroundColor() {
  return QColor(255, 240, 0, 176);
}

double Style::selectionRectRadius() {
  return 7;
}


double Style::connectionHoverWidthFactor() {
  return 4.5;
}


double Style::connectionDraftWidthFactor() {
  return 0.5;
}

QString Style::programName() {
  return "CSchem";
}

QString Style::versionName() {
  return Version::toString();
}

QFont Style::annotationFont() {
  static bool ok = false;
  static QFont lato("Lato");
  if (!ok) {
    lato.setPixelSize(21);
    //lato.setPointSize(8);
    ok = true;
  }
  return lato;
}

