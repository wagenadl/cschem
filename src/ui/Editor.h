// Editor.h

#ifndef EDITOR_H

#define EDITOR_H

#include <QWidget>
#include "data/Layout.h"
#include "data/Point.h"

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
  QSet<Point> selectedPoints() const; // not including in subgroups
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
  void selectArea(QRectF, bool add=false); // widget coordinates
  void setWidth(Dim);
  void setLayer(Layer);
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
