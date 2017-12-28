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
  void rebuild();
private:
  class SceneConnectionData *d;
};

#endif
