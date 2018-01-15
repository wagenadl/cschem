// SvgItem.h

#ifndef SVGITEM_H

#define SVGITEM_H

#include <QSvgRenderer>
#include <QGraphicsSvgItem>

class SvgItem: public QGraphicsSvgItem {
public:
  SvgItem(QGraphicsItem *parent=0);
  void setRenderer(QSharedPointer<QSvgRenderer>);
  void setSharedRenderer(QSvgRenderer *) = delete;
private:
  QSharedPointer<QSvgRenderer> r;
};

#endif
