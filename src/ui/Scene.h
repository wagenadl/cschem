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
  void setCircuit(Circuit *);
  void rebuild();
  PartLibrary const *library() const;
  Circuit const *circuit() const;
  Circuit *circuit();
  QPoint pinPosition(int partid, QString pin) const;
  void moveSelection(QPointF delta);
  void tentativelyMoveSelection(QPointF delta);
  QSet<int> selectedElements() const;
  void addConnection(int fromPart, QString fromPin, QPointF to);
  void updateOverPin(QPointF scenepos, int elt=-1, bool allowJunction=false);
  QMap<int, class SceneElement *> const &elements() const;
  QMap<int, class SceneConnection *> const &connections() const;
  void enablePinHighlighting(bool hl=true);
  int elementAt(QPointF scenepos) const;
  QString pinAt(QPointF scenepos, int elementId) const;
protected:
  void keyPressEvent(QKeyEvent *) override;
  void mousePressEvent(QGraphicsSceneMouseEvent *) override;
  void mouseMoveEvent(QGraphicsSceneMouseEvent *) override;
  void mouseReleaseEvent(QGraphicsSceneMouseEvent *) override;
private:
  void keyPressOnElement(class SceneElement *, QKeyEvent *);
  void keyPressOnConnection(class SceneConnection *, QKeyEvent *);
  void keyPressAnywhere(QKeyEvent *);
  void finalizeConnection(int fromPart, QString fromPin,
			  int toPart, QString toPin);  
private:
  class SceneData *d;
};

#endif
