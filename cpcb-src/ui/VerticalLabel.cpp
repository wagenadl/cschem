// VerticalLabel.cpp

#include "VerticalLabel.h"

#include <QPainter>

VerticalLabel::VerticalLabel(QWidget *parent): QLabel(parent) {
  ccw = false;
}

VerticalLabel::VerticalLabel(const QString &text, QWidget *parent):
  QLabel(text, parent) {
  ccw = false;
  setAutoFillBackground(true);
}

void VerticalLabel::paintEvent(QPaintEvent *) {
  QPainter painter(this);
  painter.setPen(palette().text().color());
  if (ccw) {
    painter.translate(sizeHint().width(), sizeHint().height());
    painter.rotate(270);
  } else {
    painter.rotate(90);
  }
  painter.drawText(0, 0, height(), width(), alignment(), text());
}

QSize VerticalLabel::minimumSizeHint() const {
  QSize s = QLabel::minimumSizeHint();
    return QSize(s.height(), s.width());
}

QSize VerticalLabel::sizeHint() const {
  QSize s = QLabel::sizeHint();
  return QSize(s.height(), s.width());
}
