// Scene.h

#ifndef SCENE_H

#define SCENE_H

#include <QGraphicsScene>
#include <QGraphicsItem>
#include "circuit/Circuit.h"
#include "svg/SymbolLibrary.h"

class Scene: public QGraphicsScene {
  Q_OBJECT;
public:
  Scene(SymbolLibrary *lib, QObject *parent=0);
  ~Scene();
  void setCircuit(Circuit const &);
  void setComponentValue(int eltid, QString value);
  SymbolLibrary const *library() const;
  Circuit const &circuit() const;
  Circuit &circuit();
  QPointF pinPosition(int eltid, QString pin) const;
  void moveSelection(QPoint delta, bool nomagnet);
  void tentativelyMoveSelection(QPoint delta, bool first, bool nomagnet);
  QSet<int> selectedElements() const;
  QMap<int, class SceneElement *> const &elements() const;
  QMap<int, class SceneConnection *> const &connections() const;
  int elementAt(QPointF scenepos, int exclude=-1) const;
  QString pinAt(QPointF scenepos, int elementId) const;
  // returns NOPIN if none
  int connectionAt(QPointF scenepos, int *segmentp=0) const;
  void modifyConnection(int id, QPolygonF path);
  void copyToClipboard(bool cut=false);
  void pasteFromClipboard();
  void undo();
  void redo();
  void removeDangling();
  void plonk(QString symbol, QPointF scenepos, bool merge=false);
  void makeUndoStep();
  void rotate(int dir=1);
  void flipx();
  void simplifySegment(int con, int eg);
  void unhover();
  void rehover();
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
  void libraryChanged();
private:
  class SceneData *d;
};

#endif
