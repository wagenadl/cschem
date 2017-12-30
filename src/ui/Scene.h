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
  ~Scene();
  void setCircuit(Circuit const &);
  void rebuild();
  PartLibrary const *library() const;
  Circuit const &circuit() const;
  Circuit &circuit();
  QPointF pinPosition(int partid, QString pin) const;
  void moveSelection(QPointF delta);
  void tentativelyMoveSelection(QPointF delta);
  QSet<int> selectedElements() const;
  QMap<int, class SceneElement *> const &elements() const;
  QMap<int, class SceneConnection *> const &connections() const;
  int elementAt(QPointF scenepos) const;
  QString pinAt(QPointF scenepos, int elementId) const;
  // returns "-" if none
  int connectionAt(QPointF scenepos, int *segmentp=0) const;
  void modifyConnection(int id, QPolygonF path);
protected:
  void keyPressEvent(QKeyEvent *) override;
  void keyReleaseEvent(QKeyEvent *) override;
  void mousePressEvent(QGraphicsSceneMouseEvent *) override;
  void mouseMoveEvent(QGraphicsSceneMouseEvent *) override;
  void mouseReleaseEvent(QGraphicsSceneMouseEvent *) override;
private:
  void keyPressOnElement(class SceneElement *, QKeyEvent *);
  void keyPressOnConnection(class SceneConnection *, QKeyEvent *);
  void keyPressAnywhere(QKeyEvent *);
  void finalizeConnection();
private:
  class SceneData *d;
};

#endif
