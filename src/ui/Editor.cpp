// Editor.cpp

#include "Editor.h"
#include "data/FileIO.h"
#include "data/Layout.h"
#include <QTransform>
#include <QPainter>
#include <QBrush>
#include <QPen>
#include "data/Object.h"

class EData {
public:
  EData(Editor *ed): ed(ed) {
    autofit = false;
  }
  void drawBoard(QPainter &);
  void drawGrid(QPainter &);
public:
  Editor *ed;
  Layout layout;
  QTransform mils2widget;
  QTransform widget2mils;
  bool autofit;
  QList<int> crumbs;
  QSet<int> selection;
  QSet<Point> selpts;
  struct Props {
    Dim lw;
    Layer layer;
  } props;
  Point traceStart;
  bool tracing;
};

void EData::drawBoard(QPainter &p) {
  p.setBrush(QBrush(QColor(0,0,0)));
  QPointF wtopleft = mils2widget.map(QPointF(0,0));
  double lw = layout.board().width.toMils();
  double lh = layout.board().height.toMils();
  QPointF wbotright = mils2widget.map(QPointF(lw, lh));
  p.drawRect(QRectF(wtopleft, wbotright));
}

void EData::drawGrid(QPainter &p) {
  // draw dots at either 0.1” or 2 mm intervals
  // and larger markers at either 0.5" or 10 mm intervals
  bool metric = layout.board().grid.isNull()
    ? layout.board().metric
    : layout.board().grid.isMetric();
  double lgrid = metric ? 2000/25.4 : 100;
  QPointF wgrid = mils2widget.map(QPointF(lgrid,lgrid))
    - mils2widget.map(QPointF(0,0));
  QPointF wtopleft = mils2widget.map(QPointF(0,0));
  double lw = layout.board().width.toMils();
  double lh = layout.board().height.toMils();
  QPointF wbotright = mils2widget.map(QPointF(lw, lh));
  double wgdx = wgrid.x();
  double wgdy = wgrid.y();
  double wx0 = wtopleft.x();
  double wx1 = wbotright.x();
  double wy0 = wtopleft.y();
  double wy1 = wbotright.y();
  constexpr int major = 5;
  p.setPen(QPen(QColor(255, 255, 255), 1));
  QPointF dpx(2,0);
  QPointF dpy(0,2);
  if (wgdy >= 10 && wgdy >= 10) {
    // draw everything
    for (int i=0; wx0+wgdx*i<=wx1; i++) {
      for (int j=0; wy0+wgdy*j<=wy1; j++) {
	QPointF p0(wx0+wgdx*i, wy0+wgdy*j);
	if (i%major || j%major) {
	  p.drawPoint(p0);
	} else {
	  p.drawLine(QLineF(p0 - dpx, p0 + dpx));
	  p.drawLine(QLineF(p0 - dpy, p0 + dpy));
	}
      }
    }
  } else {
    for (int i=0; wx0+wgdx*i<=wx1; i+=major) {
      for (int j=0; wy0+wgdy*j<=wy1; j+=major) {
	QPointF p0(wx0+wgdx*i, wy0+wgdy*j);
	p.drawLine(QLineF(p0 - dpx, p0 + dpx));
	p.drawLine(QLineF(p0 - dpy, p0 + dpy));
      }
    }
  }  
}

Editor::Editor(QWidget *parent): QWidget(parent), d(new EData(this)) {
  tracing = false;
  setMouseTracking(true);
  scaleToFit();
}

Editor::~Editor() {
  delete d;
}

bool Editor::load(QString fn) {
  d->layout = FileIO::loadLayout(fn);
  scaleToFit();
  return !d->layout.root().isEmpty();
}

bool Editor::save(QString fn) const {
  return FileIO::saveLayout(fn, d->layout);
}

Layout const &Editor::pcbLayout() const {
  return d->layout;
}

void Editor::scaleToFit() {
  int ww = width();
  int wh = height();
  double lw = d->layout.board().width.toMils();
  double lh = d->layout.board().height.toMils();
  constexpr int border = 10; // pix on each side
  double xfac = (ww-2*border) / (lw+1e-9);
  double yfac = (wh-2*border) / (lh+1e-9);
  double scale = (xfac < yfac) ? xfac : yfac;
  double xoffset = ww/2. - scale*lw/2.;
  double yoffset = wh/2. - scale*lh/2.;
  d->mils2widget = QTransform();
  d->mils2widget.translate(xoffset, yoffset);
  d->mils2widget.scale(scale, scale);
  d->widget2mils = d->mils2widget.inverted();
  d->autofit = true;
  update();
}

void Editor::zoomIn() {
}

void Editor::zoomOut() {
}

void Editor::resizeEvent(QResizeEvent *) {
  if (d->autofit)
    scaleToFit();
}

void Editor::mousePressEvent(QMouseEvent *) {
}

void Editor::mouseMoveEvent(QMouseEvent *e) {
  QPointF wp = e->pos();
  QPointF lp = d->widget2mils.map(wp);
  Point p = Point::fromMils(lp);
  emit hovering(p);
}

void Editor::mouseReleaseEvent(QMouseEvent *) {
}

void Editor::keyPressEvent(QKeyEvent *) {
}

void Editor::enterEvent(QEvent *) {
}

void Editor::leaveEvent(QEvent *) {
  emit leaving();
}

void Editor::paintEvent(QPaintEvent *) {
  QPainter p(this);
  d->drawBoard(p);
  d->drawGrid(p);
}

void Editor::setGrid(Dim g) {
  Board &brd = d->layout.board();  
  qDebug()<<"SetGrid" << g;
  if (g != brd.grid) {
    brd.grid = g;
    update();
    emit boardChanged(brd);
  }
}

void Editor::setLayerVisibility(Layer l, bool b) {
  Board &brd = d->layout.board();  
  if (b != brd.layervisible[l]) {
    brd.layervisible[l] = b;
    update();
    emit boardChanged(brd);
  }
}

void Editor::setPlanesVisibility(bool b) {
  Board &brd = d->layout.board();  
  if (b != brd.planesvisible) {
    brd.planesvisible = b;
    update();
    emit boardChanged(brd);
  }
}

bool Editor::enterGroup(int sub) {
  clearSelection();
  Group &here(d->layout.group(d->crumbs));
  if (here.isEmpty())
    return leaveAllGroups();
  d->crumbs << sub;
  update();
  return true;
}

bool Editor::leaveGroup() {
  clearSelection();
  if (d->crumbs.isEmpty())
    return false;
  d->crumbs.takeLast();
  update();
  return true;
}

bool Editor::leaveAllGroups() {
  clearSelection();
  if (d->crumbs.isEmpty())
    return false;
  d->crumbs.clear();
  update();
  return true;
}

void Editor::select(int id, bool add) {
  Group &here(d->layout.group(d->crumbs));
  if (!add) {
    d->selection.clear();
    d->selpts.clear();
  }
  if (here.contains(id)) {
    d->selection.insert(id);
    selectPointsOf(id);
  }
  update();
  emit selectionChanged();
}

void Editor::selectPoint(Point p, bool add) {
  if (!add) {
    d->selection.clear();
    d->selpts.clear();
  }
  d->selpts.insert(p);
  update();
  emit selectionChanged();
}

void Editor::deselect(int id) {
  if (d->selection.contains(id)) {
    d->selection.remove(id);
    update();
    emit selectionChanged();
  }
}

void Editor::deselectPoint(Point p) {
  if (d->selpts.contains(p)) {
    d->selpts.remove(p);
    update();
    emit selectionChanged();
  }
}

void Editor::selectAll() {
  Group const &here(d->layout.group(d->crumbs));
  d->selection = QSet<int>::fromList(here.keys());
  for (int id: d->selection)
    selectPointsOf(here.object(id));
  update();
  emit selectionChanged();
}

void Editor::clearSelection() {
  d->selection.clear();
  d->selpts.clear();
  update();
  emit selectionChanged();
}

void Editor::selectArea(QRectF, bool) {
  qDebug() << "selectArea NYI";
}

void Editor::selectPointsOf(int id) {
  Group const &here(d->layout.group(d->crumbs));
  if (here.contains(id))
    selectPointsOf(here.object(id));
}

void Editor::selectPointsOf(Object const &obj) {
  switch (obj.type()) {
  case Object::Type::Null:
    break;
  case Object::Type::Hole:
    d->selpts << obj.asHole().p;
    break;
  case Object::Type::Pad:
    d->selpts << obj.asPad().p;
    break;
  case Object::Type::Text:
    break;
  case Object::Type::Trace:
    d->selpts << obj.asTrace().p1 << obj.asTrace().p2;
    break;
  case Object::Type::Group:
    selectPointsOfComponent(obj.asGroup());
    break;
  }
}

void Editor::selectPointsOfComponent(Group const &g) {
  for (int id: g.keys()) {
    Object const &obj = g.object(id);
    switch (obj.type()) {
    case Object::Type::Hole:
      d->selpts << obj.asHole().p;
      break;
    case Object::Type::Pad:
      d->selpts << obj.asPad().p;
      break;
    default:
      break;
    }
  }
}

void Editor::setWidth(Dim) {
}

void Editor::setLayer(Layer) {
}
