// AbsIncToggle.cpp

#include "AbsIncToggle.h"

AbsIncToggle::AbsIncToggle(QWidget *parent):
  QToolButton(parent) {
  inc = false;
  updateIcon();
}

AbsIncToggle::~AbsIncToggle() {
}

void AbsIncToggle::setAbs() {
  inc = false;
  updateIcon();
}

void AbsIncToggle::setInc(bool i) {
  inc = i;
  updateIcon();
}
 
void AbsIncToggle::updateIcon() {
  QString ic = inc ? "Inc" : "Abs";
  setIcon(QIcon(":icons/Origin" + ic + ".svg"));
  QString balloon = inc ? "Incremental" : "Absolute";
  setToolTip(balloon + " coordinates");
}
