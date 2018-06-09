// Editor.h

#ifndef EDITOR_H

#define EDITOR_H

#include <QWidget>
#include "data/Layout.h"
#include "data/Point.h"
#include "data/Arc.h"
#include "Mode.h"
#include "EProps.h"

class Editor: public QWidget {
  Q_OBJECT;
public:
  Editor(QWidget *parent=0);
  virtual ~Editor();
  bool load(QString);
  bool save(QString) const; 
  Layout const &pcbLayout() const;
  QList<int> breadcrumbs() const; // to currently entered group
  QSet<int> selectedObjects() const; // within currently entered group
  QSet<Point> selectedPoints() const; // not including those in subgroups
  // in absolute board coordinates
  Group const &currentGroup() const;
  Point groupOffset() const;
  EProps &properties(); // for Propertiesbar to directly affect
public slots:
  void setGrid(Dim);
  void setLayerVisibility(Layer, bool);
  void setPlanesVisibility(bool);
  void scaleToFit();
  void zoomIn();
  void zoomOut();
  bool enterGroup(int sub); // sub is from current level; returns true if OK
  bool leaveGroup(); // returns true unless already at top
  bool leaveAllGroups();
  void select(int, bool add=false); // emits signal if selection changed
  void selectPoint(Point, bool add=false);
  void deselectPoint(Point);
  void deselect(int); // ditto
  void selectAll();
  void clearSelection();
  void selectArea(Rect, bool add=false);
  void setLineWidth(Dim);
  void setLayer(Layer);
  void setID(Dim);
  void setOD(Dim);
  void setWidth(Dim);
  void setHeight(Dim);
  void setSquare(bool);
  void setRef(QString);
  void setFontSize(Dim);
  void setText(QString);
  void setExtent(Arc::Extent);
  void rotateCW();
  void rotateCCW();
  void flipH();
  void flipV();
  void setRotation(int); // this is not the way to rotate or flip things
  void setFlipped(bool); // this is not the way to rotate or flip things
  void setMode(Mode);
  void formGroup();
  void dissolveGroup();
signals:
  void boardChanged(Board const &);
  void hovering(Point);
  void leaving();
  void selectionChanged();
protected:
  void mousePressEvent(QMouseEvent *) override;
  void mouseMoveEvent(QMouseEvent *) override;
  void mouseReleaseEvent(QMouseEvent *) override;
  void keyPressEvent(QKeyEvent *) override;
  void enterEvent(QEvent *) override;
  void leaveEvent(QEvent *) override;
  void paintEvent(QPaintEvent *) override;
  void resizeEvent(QResizeEvent *) override;
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
