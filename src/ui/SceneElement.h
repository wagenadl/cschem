// SceneElement.h

#ifndef SCENEELEMENT_H

#define SCENEELEMENT_H

#include <QGraphicsItemGroup>

class SceneElement: public QGraphicsItemGroup {
public:
  SceneElement(class Scene *parent, class Element const &elt);
  SceneElement(SceneElement const &) = delete;
  SceneElement &operator=(SceneElement const &) = delete;  
  ~SceneElement();
public:
  class Scene *scene();
  void showPins();
  void hidePins();
  void showName();
  void hideName();
  void showValue();
  void hideValue();
  void showLabel();
  void hideLabel();
  void rebuild();
protected:
  void hoverEnterEvent(QGraphicsSceneHoverEvent *) override;
  void hoverLeaveEvent(QGraphicsSceneHoverEvent *) override;
  QVariant itemChange(GraphicsItemChange, const QVariant &) override;
  void mousePressEvent(QGraphicsSceneMouseEvent *) override;
  void mouseMoveEvent(QGraphicsSceneMouseEvent *) override;
  void mouseReleaseEvent(QGraphicsSceneMouseEvent *) override;
private:
  class SceneElementData *d;
};

#endif
