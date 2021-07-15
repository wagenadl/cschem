// Editor.h

#ifndef EDITOR_H

#define EDITOR_H

#include <QWidget>
#include "data/Layout.h"
#include "data/Point.h"
#include "data/Arc.h"
#include "Mode.h"
#include "EProps.h"
#include "data/LinkedSchematic.h"

class Editor: public QWidget {
  Q_OBJECT;
public:
  Editor(QWidget *parent=0);
  virtual ~Editor();
  bool load(QString);
  bool save(QString); 
  Layout const &pcbLayout() const;
  NodeID breadcrumbs() const; // to currently entered group
  QSet<int> selectedObjects() const; // within currently entered group
  QSet<Point> selectedPoints() const; // not including those in subgroups
  // in absolute board coordinates
  Group const &currentGroup() const;
  EProps &properties(); // for Propertiesbar to directly affect
  int selectedComponent(QString *msg=0) const;
  // returns ID if one group, or 0 if none, in which case msg says why
  bool linkSchematic(QString fn);
  void unlinkSchematic();
  LinkedSchematic const &linkedSchematic() const;
  Point hoverPoint() const;
  bool isUndoAvailable() const;
  bool isRedoAvailable() const;
  bool isAsSaved() const;
  double pixelsPerMil() const;
  class PlaneEditor *planeEditor() const;
  Point userOrigin() const;
  class BOM *bom() const;
  bool loadBOM(QString fn);
public slots:
  void setAngleConstraint(bool);
  void setGrid(Dim);
  void setLayerVisibility(Layer, bool);
  void setPlanesVisibility(bool);
  void setNetsVisibility(bool);
  void scaleToFit();
  void zoomIn();
  void zoomOut();
  void doubleClickOn(Point p, int id);
  bool enterGroup(int sub); // sub is from current level; returns true if OK
  bool leaveGroup(); // returns true unless already at top
  bool leaveAllGroups();
  void select(int, bool add=false); // emits signal if selection changed
  void selectPoint(Point, bool add=false);
  void deselectPoint(Point);
  void deselect(int); // ditto
  void selectAll();
  void selectTrace(bool wholetree);
  // expand selection with rest of multisegment trace
  void clearSelection();
  void selectArea(Rect, bool add=false);
  void setLineWidth(Dim);
  void setLayer(Layer);
  void setID(Dim);
  void setOD(Dim);
  void setSlotLength(Dim);
  void setWidth(Dim);
  void setHeight(Dim);
  void setSquare(bool);
  void setRefText(QString);
  void setFontSize(Dim);
  void setArcAngle(int angle);
  void rotateCW(bool noundo=false);
  void rotateCCW(bool noundo=false);
  void arbitraryRotation(int angleCW);
  void flipH(bool noundo=false);
  void flipV();
  void setRotation(int); // this is not the way to rotate or flip things
  void setFlipped(bool); // this is not the way to rotate or flip things
  void setMode(Mode);
  void setGroupPackage(QString);
  void setGroupPartno(QString);
  void setGroupNotes(QString);
  void setGroupPackage(NodeID, QString);
  void setGroupPartno(NodeID, QString);
  void setGroupNotes(NodeID, QString);
  void formGroup();
  void dissolveGroup();
  void deleteSelected();
  bool saveComponent(int id, QString fn); // true if OK.
  // id must be in current group (see breadcrumbs). Updates "pkg" of
  // saved group
  bool insertComponent(QString fn, Point pt); // true if OK.
  void undo();
  void redo();
  void cut();
  void copy();
  void paste();
  void markAsSaved();
  void translate(Point);
  void updateOnNet();
  void pretendOnNet(NodeID);
  void deleteDanglingTraces();
  void cleanupIntersections();
  void setBoardSize(Dim w, Dim h, Board::Shape shp=Board::Shape::Rect);
signals:
  void userOriginChanged(Point);
  void insertedPadOrHole();
  void boardChanged(Board const &);
  void hovering(Point);
  void onObject(QString);
  void missingNodes(QStringList);
  void leaving();
  void selectionChanged(bool); // true if any objects other than pure points
                               // are selected
  void componentsChanged(); // emitted when a component has been placed,
                            // created, removed, or renamed
  void changedFromSaved(bool);
  void undoAvailable(bool);
  void redoAvailable(bool);
  void schematicLinked(bool);
  void selectionIsGroup(bool);
  void scaleChanged();
  void tentativeMove(Point delta);
protected:
  void mouseDoubleClickEvent(QMouseEvent *) override;
  void mousePressEvent(QMouseEvent *) override;
  void mouseMoveEvent(QMouseEvent *) override;
  void mouseReleaseEvent(QMouseEvent *) override;
  void keyPressEvent(QKeyEvent *) override;
  void enterEvent(QEvent *) override;
  void leaveEvent(QEvent *) override;
  void paintEvent(QPaintEvent *) override;
  void resizeEvent(QResizeEvent *) override;
  void wheelEvent(QWheelEvent *) override;
  void dragEnterEvent(QDragEnterEvent *) override;
  void dragLeaveEvent(QDragLeaveEvent *) override;
  void dragMoveEvent(QDragMoveEvent *) override;
  void dropEvent(QDropEvent *) override;
private:
  void selectPointsOf(int id);
  void selectPointsOf(Object const &obj);
  // SELECTPOINTSOF adds points of Trace, Hole, Pad, Component (Group).
  void selectPointsOfComponent(Group const &);
  // SELECTPOINTSOFCOMPONENT adds points of holes and pads in group, but
  // not of contained traces and subgroups.
private:
  class EData *d;
};

#endif
