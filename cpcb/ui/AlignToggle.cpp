// AlignToggle.cpp

#include "AlignToggle.h"

AlignToggle::AlignToggle(Qt::Orientation o, QWidget *parent):
  QToolButton(parent), o(o), a(AlignToggle::Pin) {
  connect(this, &QAbstractButton::clicked,
          this, &AlignToggle::click);
  updateIcon();
}

AlignToggle::~AlignToggle() {
}

void AlignToggle::setOrientation(Qt::Orientation o1) {
  o = o1;
  updateIcon();
}

void AlignToggle::setAlignment(Alignment a1) {
  a = a1;
  updateIcon();
  emit alignmentChanged(a);
}
 
void AlignToggle::updateIcon() {
  QString ic = (o == Qt::Horizontal)
    ? ((a == Min) ? "Left"
       : (a == Max) ? "Right"
       : (a == Center) ? "HCenter"
       : "HPin")
    : ((a == Min) ? "Top"
       : (a == Max) ? "Bottom"
       : (a == Center) ? "VCenter"
       : "VPin");
  setIcon(QIcon(":icons/Align" + ic + ".svg"));
  QString balloon = (o == Qt::Horizontal)
    ? ((a == Min) ? "left edge"
       : (a == Max) ? "right edge"
       : (a == Center) ? "center"
       : "principal pin")
    : ((a == Min) ? "top edge"
       : (a == Max) ? "bottom edge"
       : (a == Center) ? "center"
       : "principal pin");
  setToolTip("Position on " + balloon + " of selection");
}

void AlignToggle::click() {
  if (a==Min)
    a = Center;
  else if (a==Center)
    a = Max;
  else if (a==Max)
    a = Pin;
  else
    a = Min;
  updateIcon();
  emit alignmentChanged(a);
}

Dim AlignToggle::extractDimension(Rect const &r, Point const &pin) {
  return extractDimension(r, pin, o, a);
}

Dim AlignToggle::extractDimension(Rect const &r, Point const &pin,
                                  Qt::Orientation o, Alignment a) {
  switch (a) {
  case Min:
    return (o==Qt::Horizontal) ? r.left : r.top;
  case Center:
    return (o==Qt::Horizontal) ? r.center().x : r.center().y;
  case Max:
    return (o==Qt::Horizontal) ? r.right() : r.bottom();
  case Pin:
    return (o==Qt::Horizontal) ? pin.x : pin.y;
  default:
    qWarning("Unknown alignment state");
    return Dim();
  }
}
