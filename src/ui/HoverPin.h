// HoverPin.h

#ifndef HOVERPIN_H

#define HOVERPIN_H

#include <QGraphicsEllipseItem>

class HoverPin: public QGraphicsEllipseItem {
public:
  HoverPin(class Scene *parent);
  void updateHover(QPointF scenepos, int elt=-1);
  /* elt<0 means: need to figure it out.
     elt=0 means: no element
     elt>0 means: element known
  */
private:
  class Scene *scene;
  int elt;
  QString pin;
};

#endif
