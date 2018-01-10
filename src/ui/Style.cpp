// Style.cpp

#include "Style.h"

QColor Style::textColor() {
  return QColor(0, 0, 0);
}

QColor Style::faintColor() {
  return QColor(180, 180, 180);
}

QColor Style::connectionColor() {
  return QColor(0, 0, 0);
}


QColor Style::danglingColor() {
  return QColor(255, 0, 0);
}


QColor Style::elementHoverColor() {
  return QColor(64, 192, 255);
}

QColor Style::selectedElementHoverColor() {
  return QColor(0, 64, 255);
}


QColor Style::connectionHoverColor() {
  return QColor(64, 192, 255);
}


QColor Style::pinHighlightColor() {
  return QColor(0, 255, 128);
}

QColor Style::selectionColor() {
  return QColor(255, 240, 0, 176);
}

double Style::selectionRectRadius() {
  return 7;
}


double Style::connectionHoverWidthFactor() {
  return 3;
}


double Style::connectionDraftWidthFactor() {
  return 0.5;
}

QString Style::programName() {
  return "CSchem";
}

QString Style::versionName() {
  return "1.0.0";
}

QFont Style::annotationFont() {
  return QFont("Lato", 8);
}

