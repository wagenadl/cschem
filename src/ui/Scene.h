// Scene.h

#ifndef SCENE_H

#define SCENE_H

#include <QGraphicsScene>
#include <QGraphicsItem>
#include "circuit/Schem.h"
#include "svg/SymbolLibrary.h"

class Scene: public QGraphicsScene {
  Q_OBJECT;
public:
  Scene(Schem const &schem, QObject *parent=0);
  ~Scene();
  void updateFromPartList(Element const &);
  SymbolLibrary const &library() const;
  Circuit const &circuit() const;
  Schem const &schem() const;
  QPointF pinPosition(int eltid, QString pin) const;
  QPointF pinPosition(PinID pid) const { return pinPosition(pid.element(), pid.pin()); }
  void moveSelection(QPoint delta, bool nomagnet);
  void tentativelyMoveSelection(QPoint delta, bool first, bool nomagnet);
  QSet<int> selectedElements() const;
  void selectElements(QSet<int> const &);
  QMap<int, class SceneElement *> const &elements() const;
  QMap<int, class SceneConnection *> const &connections() const;
  int elementAt(QPointF scenepos, int exclude=-1) const;
  QString pinAt(QPointF scenepos, int elementId) const;
  // returns NOPIN if none
  int connectionAt(QPointF scenepos, int *segmentp=0) const;
  void modifyConnection(int id, QPolygonF path);
  void modifyElementAnnotations(Element const &);
  /* This can change name, value, info, and position and visibility of labels,
     but *not* element type, position, etc. It also affects annotation in
     other elements that are related by containership. */
  void renumber(QMap<int, QString> const &map); // renumber some elements
  void copyToClipboard(bool cut=false);
  void pasteFromClipboard();
  void undo();
  void redo();
  void removeDangling();
  void plonk(QString symbol, QPointF scenepos, bool merge=false);
  void rotate(int dir=1);
  void flipx();
  void simplifySegment(int con, int eg);
  void unhover();
  void rehover();
  class PartList *partlist() const;
  class HoverManager *hoverManager() const;
  void clearSelection();
  void perhapsEmitSelectionChange();
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
signals:
  void libraryChanged();
  void circuitChanged();
private:
  void emitCircuitChanged();
private:
  class SceneData *d;
  friend class SceneData;
};

#endif
