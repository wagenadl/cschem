// SceneConnection.h

#ifndef SCENECONNECTION_H

#define SCENECONNECTION_H

#include <QGraphicsItem>

class SceneConnection: public QGraphicsItem {
public:
  SceneConnection(class Scene *parent, class Connection const &);
  SceneConnection(SceneConnection const &) = delete;
  SceneConnection &operator=(SceneConnection const &) = delete;  
  ~SceneConnection();
public:
  class Scene *scene();
  int id() const;
  void rebuild();
  void temporaryTranslate(QPointF delta);
  void temporaryTranslateFrom(QPointF delta);
  void temporaryTranslateTo(QPointF delta);
  void setLineWidth(double frac = 1.0);
public:
  void paint(QPainter *, QStyleOptionGraphicsItem const *, QWidget *) override;
  QRectF boundingRect() const override;  
private:
  void setPath(class QPolygonF const &);
private:
  class SceneConnectionData *d;
};

#endif
