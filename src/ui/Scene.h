// Scene.h

#ifndef SCENE_H

#define SCENE_H

#include <QGraphicsScene>
#include <QGraphicsItem>
#include "file/Circuit.h"
#include "svg/PartLibrary.h"

class Scene: public QGraphicsScene {
  Q_OBJECT;
public:
  Scene(PartLibrary const *lib, QObject *parent=0);
  ~Scene();
  void setCircuit(Circuit const &);
  void setComponentValue(int eltid, QString value);
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
  void copyToClipboard(bool cut=false);
  void pasteFromClipboard();
  void undo();
  void redo();
  void removeDangling();
  void plonk(QString symbol, QPointF scenepos);
  void makeUndoStep();
  void rotate(int dir=1);
  void flipx();
  void exportPdf(QString file);
protected:
  void keyPressEvent(QKeyEvent *) override;
  void keyReleaseEvent(QKeyEvent *) override;
  void mousePressEvent(QGraphicsSceneMouseEvent *) override;
  void mouseMoveEvent(QGraphicsSceneMouseEvent *) override;
  void mouseReleaseEvent(QGraphicsSceneMouseEvent *) override;
  void dragEnterEvent(QGraphicsSceneDragDropEvent *) override;
  void dragLeaveEvent(QGraphicsSceneDragDropEvent *) override;
  void dragMoveEvent(QGraphicsSceneDragDropEvent *) override;
  void dropEvent(QGraphicsSceneDragDropEvent *) override;
  void focusInEvent(QFocusEvent *) override;
  void focusOutEvent(QFocusEvent *) override;
public:
  void annotationInternallyEdited(int id);
signals:
  void annotationEdited(int id);
private:
  class SceneData *d;
};

#endif
