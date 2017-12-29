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
  int id() const;
  void showName();
  void hideName();
  void showValue();
  void hideValue();
  void showLabel();
  void hideLabel();
  void rebuild();
protected:
  void mousePressEvent(QGraphicsSceneMouseEvent *) override;
  void mouseMoveEvent(QGraphicsSceneMouseEvent *) override;
  void mouseReleaseEvent(QGraphicsSceneMouseEvent *) override;
private:
  class SceneElementData *d;
};

#endif
