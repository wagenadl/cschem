// SceneElementAnnotation.h

#ifndef SCENEELEMENTANNOTATION_H

#define SCENEELEMENTANNOTATION_H

#include <QGraphicsTextItem>

// This is an annotation for a scene element, not a free annotation

class SceneElementAnnotation: public QGraphicsTextItem {
  Q_OBJECT;
public:
  SceneElementAnnotation(double movestep, QGraphicsItem *parent=0);
  virtual ~SceneElementAnnotation();
  SceneElementAnnotation(SceneElementAnnotation const &) = delete;
  SceneElementAnnotation &operator=(SceneElementAnnotation const &) = delete;
public:
  void backspace();
  void setCenter(QPointF xy); // set position based on center of bbox
  void forceHoverColor(bool);
  void markSelected(bool);
  void setFaint(bool);
  void setPlaceholderText(QString);
  void paint(QPainter *, QStyleOptionGraphicsItem const *, QWidget *) override;
signals:
  void returnPressed();
  void escapePressed();
  void moved(QPointF delta);
  void removalRequested();
  void hovering(bool);
  void focused();
protected:
  void keyPressEvent(QKeyEvent *) override;
  void mousePressEvent(QGraphicsSceneMouseEvent *) override;
  void mouseMoveEvent(QGraphicsSceneMouseEvent *) override;
  void mouseReleaseEvent(QGraphicsSceneMouseEvent *) override;
  void focusInEvent(QFocusEvent *) override;
  void focusOutEvent(QFocusEvent *) override;
  void hoverEnterEvent(QGraphicsSceneHoverEvent *) override;
  void hoverLeaveEvent(QGraphicsSceneHoverEvent *) override;
private slots:
  void updateCenter();
private:
  void setPos(QPointF const &); // use setCenter instead!
private:
  class SAData *d;
};

#endif
