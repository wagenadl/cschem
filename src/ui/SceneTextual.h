// SceneTextual.h

#ifndef SCENETEXTUAL_H

#define SCENETEXTUAL_H

#include <QGraphicsTextItem>

class SceneTextual: public QGraphicsTextItem {
public:
  SceneTextual(class Scene *parent, class Textual const &txt);
  SceneTextual(SceneTextual const &) = delete;
  SceneTextual &operator=(SceneTextual const &) = delete;  
  ~SceneTextual();
public:
  class Scene *scene();
  int id() const;
  void setTextual(Textual const &txt);
  //  void rebuild();
  //  void hover();
  //  void unhover();
  void paint(QPainter *, QStyleOptionGraphicsItem const *, QWidget *) override;
  void temporaryTranslate(QPoint delta);
  void setSelected(bool);
  bool isSelected() const;
protected:
  void keyPressEvent(QKeyEvent *) override;
  void hoverEnterEvent(QGraphicsSceneHoverEvent *) override;
  void hoverLeaveEvent(QGraphicsSceneHoverEvent *) override;
  void mousePressEvent(QGraphicsSceneMouseEvent *) override;
  void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *) override;
  void mouseMoveEvent(QGraphicsSceneMouseEvent *) override;
  void mouseReleaseEvent(QGraphicsSceneMouseEvent *) override;
private:
  class STData *d;
};

#endif
