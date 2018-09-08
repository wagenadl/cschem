// SvgItem.cpp

#include "SvgItem.h"

SvgItem::SvgItem(QGraphicsItem *parent): QGraphicsSvgItem(parent) {
}

void SvgItem::setRenderer(QSharedPointer<QSvgRenderer> r0) {
  r = r0;
  if (!r.data())
    r = QSharedPointer<QSvgRenderer>(new QSvgRenderer);
  QGraphicsSvgItem::setSharedRenderer(r.data());
}
