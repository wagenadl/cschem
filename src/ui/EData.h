// EData.h

#ifndef EDATA_H

#define EDATA_H

#include "Editor.h"

#include "data/PCBFileIO.h"
#include "data/Layout.h"
#include "data/Orient.h"
#include "data/Object.h"
#include "ORenderer.h"
#include "data/LinkedSchematic.h"
#include "ComponentView.h"
#include "ElementView.h"
#include "data/UndoStep.h"
#include "data/Clipboard.h"
#include "data/PCBNet.h"
#include "data/LinkedNet.h"
#include "data/NetMismatch.h"

#include <QTransform>
#include <QPainter>
#include <QBrush>
#include <QPen>
#include <QRubberBand>
#include <QInputDialog>
#include <QMimeData>
#include <QFileInfo>

class EData {
public:
  EData(Editor *ed): ed(ed) {
    autofit = false;
    mode = Mode::Edit;
    tracing = false;
    moving = false;
    panning = false;
    rubberband = 0;
    stuckptsvalid = false;
    stepsfromsaved = false;
    netsvisible = true;
  }
  void drawBoard(QPainter &) const;
  void drawGrid(QPainter &) const;
  void drawSelectedPoints(QPainter &) const;
  void drawObjects(QPainter &) const;
  void drawObject(Object const &o, Layer l, Point const &origin,
		  QPainter &p, bool selected, bool toplevel=false) const;
  // only draw parts of object that are part of given layer
  void drawNetMismatch(class ORenderer &) const;
  void drawTracing(QPainter &) const;
  void pressEdit(Point, Qt::KeyboardModifiers);
  int visibleObjectAt(Point p, Dim mrg=Dim()) const;
  void pressPad(Point);
  void pressArc(Point);
  void pressHole(Point);
  void pressText(Point);
  void pressTracing(Point);
  void pressPickingUp(Point);
  void moveTracing(Point);
  void abortTracing();
  void moveBanding(Point);
  void moveMoving(Point);
  void releaseBanding(Point);
  void releaseMoving(Point);
  void pressPanning(QPoint);
  void movePanning(QPoint);
  void dropFromSelection(int id, Point p, Dim mrg);
  void startMoveSelection();
  void newSelectionUnless(int id, Point p, Dim mrg, bool add);
  void selectPointsOf(int id);
  QSet<Point> pointsOf(Object const &obj) const;
  QSet<Point> pointsOf(Object const &obj, Layer lay) const;
  Rect selectionBounds() const; // board coordinates
  void validateStuckPoints() const;
  void invalidateStuckPoints() const;
  void zoom(double factor);
  void createUndoPoint();
  bool updateOnWhat(bool force=false);
  void updateNet(NodeID seed);
  void emitSelectionStatus();
public:
  Editor *ed;
  Layout layout;
  QTransform mils2widget;
  QTransform widget2mils;
  double mils2px;
  bool autofit;
  bool netsvisible;
  QList<int> crumbs;
  QSet<int> selection;
  QMap<Layer, QSet<Point> > selpts; // selected points that *are* part
  // of a selected object, by layer
  QMap<Layer, QSet<Point> > purepts; // selected points that are *not*
  // part of any sel. object, by layer
  mutable bool stuckptsvalid; // indicator: if false, stuckpts needs to be
  // recalced.
  mutable QMap<Layer, QSet<Point> > stuckpts; // points that are part of a
  // selected object but also of a non-trace non-selected object and that
  // should not move
  EProps props;
  Point tracestart;
  Point tracecurrent;
  Point presspoint;
  Point movingstart;
  Point hoverpt;
  bool tracing;
  bool moving;
  bool panning;
  Point movingdelta;
  Mode mode;
  QRubberBand *rubberband = 0;
  QTransform pan0;
  QPoint panstart;
  LinkedSchematic linkedschematic;
  QList<UndoStep> undostack;
  QList<UndoStep> redostack;
  int stepsfromsaved;
  QString onobject;
  NodeID onnode;
  PCBNet net;
  LinkedNet linkednet;
  NetMismatch netmismatch;
};


#endif
