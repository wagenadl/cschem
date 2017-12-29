// HoverPin.h

#ifndef HOVERPIN_H

#define HOVERPIN_H

#include <QGraphicsEllipseItem>

class HoverPin: public QGraphicsEllipseItem {
public:
  HoverPin(class Scene *parent);
  void updateHover(QPointF scenepos, int elt=-1, bool allowJunction=false);
  /* elt<0 means: need to figure it out.
     elt=0 means: no element
     elt>0 means: element known
  */
  int element() const { return elt; }
  QString pinName() const { return pin; }
private:
  class Scene *scene;
  int elt;
  QString pin;
};

#endif
