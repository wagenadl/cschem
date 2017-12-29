// SceneConnection.h

#ifndef SCENECONNECTION_H

#define SCENECONNECTION_H

#include <QGraphicsPathItem>

class SceneConnection: public QGraphicsPathItem {
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
protected:
  void hoverEnterEvent(QGraphicsSceneHoverEvent *) override;
  void hoverLeaveEvent(QGraphicsSceneHoverEvent *) override;
private:
  class SceneConnectionData *d;
};

#endif
