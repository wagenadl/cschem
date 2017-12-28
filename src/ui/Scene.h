// Scene.h

#ifndef SCENE_H

#define SCENE_H

#include <QGraphicsScene>
#include <QGraphicsItem>
#include "file/Circuit.h"
#include "svg/PartLibrary.h"

class Scene: public QGraphicsScene {
public:
  Scene(PartLibrary const *lib, QObject *parent=0);
  void setCircuit(Circuit *);
  void rebuild();
  PartLibrary const *library() const;
  Circuit const *circuit() const;
  Circuit *circuit();
  QPoint pinPosition(int partid, QString pin) const;
  void moveSelection(QPointF delta);
private:
  PartLibrary const *lib;
  Circuit *circ;
  QMap<int, class SceneElement *> elts;
  QMap<int, class SceneConnection *> conns;
};

#endif
