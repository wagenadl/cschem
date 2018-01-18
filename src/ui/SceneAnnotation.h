// SceneAnnotation.h

#ifndef SCENEANNOTATION_H

#define SCENEANNOTATION_H

#include <QGraphicsTextItem>

class SceneAnnotation: public QGraphicsTextItem {
  Q_OBJECT;
public:
  SceneAnnotation(double movestep, QGraphicsItem *parent=0);
  virtual ~SceneAnnotation();
  SceneAnnotation(SceneAnnotation const &) = delete;
  SceneAnnotation &operator=(SceneAnnotation const &) = delete;
public:
  void backspace();
  void setBaseline(QPointF);
signals:
  void returnPressed();
  void escapePressed();
  void moved(QPointF delta);
  void removalRequested();
  void hovering(bool);
protected:
  void keyPressEvent(QKeyEvent *) override;
  void mousePressEvent(QGraphicsSceneMouseEvent *) override;
  void mouseMoveEvent(QGraphicsSceneMouseEvent *) override;
  void mouseReleaseEvent(QGraphicsSceneMouseEvent *) override;
  void focusInEvent(QFocusEvent *) override;
  void focusOutEvent(QFocusEvent *) override;
  void hoverEnterEvent(QGraphicsSceneHoverEvent *) override;
  void hoverLeaveEvent(QGraphicsSceneHoverEvent *) override;
private:
  class SAData *d;
};

#endif
