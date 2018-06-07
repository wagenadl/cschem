// Editor.cpp

#include "Editor.h"
#include "data/FileIO.h"
#include "data/Layout.h"
#include <QTransform>
#include <QPainter>
#include <QBrush>
#include <QPen>
#include "data/Object.h"
#include <QRubberBand>

class EData {
public:
  EData(Editor *ed): ed(ed) {
    autofit = false;
    mode = Mode::Edit;
    tracing = false;
    moving = false;
    rubberband = 0;
  }
  void drawBoard(QPainter &) const;
  void drawGrid(QPainter &) const;
  void drawSelectedPoints(QPainter &) const;
  void drawObjects(QPainter &) const;
  void drawObject(Object const &o, Layer l, Point const &origin,
		  QPainter &p, bool selected, bool toplevel=false) const;
  // only draw parts of object that are part of given layer
  void drawPlanes(Layer l, QPainter &p) const;
  void drawTracing(QPainter &) const;
  void pressEdit(Point, Qt::KeyboardModifiers);
  int visibleObjectAt(Point p, Dim mrg=Dim()) const;
  void pressTracing(Point);
  void moveTracing(Point);
  void abortTracing();
  void moveBanding(Point);
  void moveMoving(Point);
  void releaseBanding(Point);
  void releaseMoving(Point);
  void dropFromSelection(int id, Point p, Dim mrg);
  void startMoveSelection();
  void newSelectionUnless(int id, Point p, Dim mrg, bool add);
  void selectPointsOf(int id);
  void selectPointsOf(Object const &obj);
  void selectPointsOfComponent(Group const &g);
public:
  Editor *ed;
  Layout layout;
  QTransform mils2widget;
  QTransform widget2mils;
  double mils2px;
  bool autofit;
  QList<int> crumbs;
  QSet<int> selection;
  QSet<Point> selpts; // selected points that *are* part of a selected object
  QSet<Point> purepts; // selected points that are *not* part of any sel. object
  struct Props {
    Dim linewidth;
    Layer layer;
  } props;
  Point tracestart;
  Point tracecurrent;
  Point presspoint;
  Point movingstart;
  bool tracing;
  bool moving;
  Point movingdelta;
  Mode mode;
  QRubberBand *rubberband = 0;
};

void EData::selectPointsOf(int id) {
  Group const &here(layout.root().subgroup(crumbs));
  if (here.contains(id))
    selectPointsOf(here.object(id));
}

void EData::selectPointsOf(Object const &obj) {
  switch (obj.type()) {
  case Object::Type::Null:
    break;
  case Object::Type::Hole:
    selpts << obj.asHole().p;
    break;
  case Object::Type::Pad:
    selpts << obj.asPad().p;
    break;
  case Object::Type::Text:
    break;
  case Object::Type::Trace:
    selpts << obj.asTrace().p1 << obj.asTrace().p2;
    break;
  case Object::Type::Group:
    selectPointsOfComponent(obj.asGroup());
    break;
  }
}

void EData::selectPointsOfComponent(Group const &g) {
  for (int id: g.keys()) {
    Object const &obj = g.object(id);
    switch (obj.type()) {
    case Object::Type::Hole:
      selpts << obj.asHole().p;
      break;
    case Object::Type::Pad:
      selpts << obj.asPad().p;
      break;
    default:
      break;
    }
  }
}

void EData::drawBoard(QPainter &p) const {
  p.setBrush(QBrush(QColor(0,0,0)));
  QPointF wtopleft = mils2widget.map(QPointF(0,0));
  double lw = layout.board().width.toMils();
  double lh = layout.board().height.toMils();
  QPointF wbotright = mils2widget.map(QPointF(lw, lh));
  p.drawRect(QRectF(wtopleft, wbotright));
}

void EData::drawGrid(QPainter &p) const {
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

void EData::drawTracing(QPainter &p) const {
  if (!tracing)
    return;
  qDebug() << "props" << int(props.layer) << props.linewidth;
  qDebug() << "drawtracing" << layerColor(props.layer)
	   << props.linewidth.toMils()*mils2px
	   << mils2widget.map(tracestart.toMils())
	   << mils2widget.map(tracecurrent.toMils());
  p.setPen(QPen(layerColor(props.layer), props.linewidth.toMils()*mils2px));
  p.drawLine(mils2widget.map(tracestart.toMils()),
	     mils2widget.map(tracecurrent.toMils()));
}

void EData::drawPlanes(Layer, QPainter &) const {
  // Bottom filled plane is easy.
  // Top filled plane is tricky because holes should let bottom shine through.
  // The full solution is to render the plane into a pixmap first, then
  // copy to the widget.
  // An alternative is to ADD the plane to the widget, then SUBTRACT
  // the holes, using appropriate color scheme. That would let bottom traces
  // shine through, but I am OK with that.
}

void EData::drawObjects(QPainter &p) const {
  Group const &here(layout.root().subgroup(crumbs));
  Point origin = layout.root().originOf(crumbs);
  auto onelayer = [&](Layer l) {
    for (int id: here.keys())
      drawObject(here.object(id), l, origin, p, selection.contains(id), true);
  };
  Board const &brd = layout.board();
  if (brd.layervisible[Layer::Bottom]) {
    if (brd.planesvisible)
      drawPlanes(Layer::Bottom, p);
    onelayer(Layer::Bottom);
  }
  if (brd.layervisible[Layer::Top]) {
    if (brd.planesvisible)
      drawPlanes(Layer::Top, p);
    onelayer(Layer::Top);
  }
  if (brd.layervisible[Layer::Silk])
    onelayer(Layer::Silk);
}

void EData::drawObject(Object const &o, Layer l,
		       Point const &origin, QPainter &p,
		       bool selected, bool toplevel) const {
  /* In toplevel *only*, and only during moves, selected objects are
     translated by the movingdelta and nonselected traces with
     selected endpoints have those endpoints translated.
   */
  switch (o.type()) {
  case Object::Type::Trace: {
    Trace const &t = o.asTrace();
    if (t.layer==l) {
      p.setPen(QPen(layerColor(t.layer, selected), t.width.toMils()*mils2px));
      Point p1 = origin + t.p1;
      Point p2 = origin + t.p2;
      if (moving && toplevel) {
	if (selected) {
	  p1 += movingdelta;
	  p2 += movingdelta;
	} else {
	  if (selpts.contains(t.p1) || purepts.contains(t.p1))
	    p1 += movingdelta;
	  if (selpts.contains(t.p2) || purepts.contains(t.p2))
	    p2 += movingdelta;
	}
      }
      p.drawLine(mils2widget.map(p1.toMils()),
		 mils2widget.map(p2.toMils()));
    }
  } break;
  case Object::Type::Group: {
    Group const &g = o.asGroup();
    Point ori = origin + g.origin;
    if (selected && toplevel && moving)
      ori += movingdelta;
    for (int id: g.keys())
      drawObject(g.object(id), l, ori, p, selected);
  } break;
  default:
    break;
  }
}

void EData::drawSelectedPoints(QPainter &p) const {
  if (purepts.isEmpty())
    return;
  p.setPen(QPen(Qt::NoPen));
  p.setBrush(QColor(200, 200, 200));
  double r = mils2px * 25;
  for (Point pt: purepts) {
    if (moving)
      pt += movingdelta;
    QPointF c = mils2widget.map(pt.toMils());
    p.drawEllipse(c, r, r);
  }
}

void EData::abortTracing() {
  tracing = false;
  ed->update();
}

void EData::pressTracing(Point p) {
  p = p.roundedTo(layout.board().grid);
  if (p == tracestart) {
    abortTracing();
    return;
  }
  qDebug() << "tracing from" << p;
  if (tracing) {
    Group &here(layout.root().subgroup(crumbs));
    Trace t;
    t.p1 = tracestart;
    t.p2 = p;
    t.width = props.linewidth;
    t.layer = props.layer;
    here.insert(Object(t));
  } else {
    tracing = true;
  }
  tracestart = p;
  tracecurrent = p;
  ed->update();
}

void EData::moveMoving(Point p) {
  movingdelta = p.roundedTo(layout.board().grid) - movingstart;
  ed->update();
}

void EData::moveTracing(Point p) {
  tracecurrent = p.roundedTo(layout.board().grid);
  qDebug() << "movetracing to" << tracecurrent;
  ed->update();
}

enum class Prio {
  None,
  BottomPlane,
  TopPlane,
  BottomTrace,
  BottomObject,
  TopTrace,
  TopObject,
  Silk
};

int EData::visibleObjectAt(Point p, Dim mrg) const {
  QList<int> ids = layout.root().objectsAt(p, mrg);
  qDebug() << ids;
  /* Now, we want to select one item that P is on.
     We prioritize higher layers over lower layers, ignore pads, text, traces
     on hidden layers, prioritize holes, pads, groups [components], text over
     traces, which are prioritized over planes.
     If P is on an endpoint of a segment, we need to know about that as well.
  */
  int fave = -1;
  Prio prio = Prio::None;
  Board const &brd = layout.board();
  auto better = [prio](Prio p1) { return int(p1) > int(prio); };
  for (int id: ids) {
    Prio p1 = Prio::None;
    Object const &obj = layout.root().object(id);
    Layer l = obj.layer();
    switch (obj.type()) {
      /* Planes NYI
    case Object::Type::Plane:
      if (layervisible[l])
	p1 = l==Layer::Bottom ? Prio::BottomPlane : Prio::TopPlane;
    break;
      */
    case Object::Type::Trace:
      if (brd.layervisible[l])
	p1 = l==Layer::Bottom ? Prio::BottomTrace
	  : l==Layer::Top ? Prio::TopTrace
	  : Prio::Silk;
      break;
    case Object::Type::Text: case Object::Type::Pad:
      if (brd.layervisible[l])
	p1 = l==Layer::Bottom ? Prio::BottomObject
	  : l==Layer::Top ? Prio::TopObject
	  : Prio::Silk;
      break;
    case Object::Type::Hole:
      if (brd.layervisible[Layer::Top])
	p1 = Prio::TopObject;
      else if (brd.layervisible[Layer::Bottom])
	p1 = Prio::BottomObject;
      break;
    case Object::Type::Group:
      p1 = Prio::Silk;
      break;
    default:
      break;
    }
    if (better(p1)) {
      prio = p1;
      fave = id;
    }
  }
  return fave;
}

void EData::pressEdit(Point p, Qt::KeyboardModifiers m) {
  qDebug() << "pressedit";
  Dim mrg = Dim::fromMils(4/mils2px);
  int fave = visibleObjectAt(p, mrg);
  bool add = m & Qt::ShiftModifier;
  if (fave < 0) {
    // not on anything -> start rectangle select
    if (!add)
      ed->clearSelection();
    if (!rubberband)
      rubberband = new QRubberBand(QRubberBand::Rectangle, ed);
    rubberband->show();
    rubberband->setGeometry(QRectF(mils2widget.map(p.toMils()), QSize(0,0))
			    .toRect());
  } else {
    qDebug() << "fave" << fave;
    if (selection.contains(fave)) {
      if (add) {
	dropFromSelection(fave, p, mrg);
      } else {
	startMoveSelection();
      }
    } else {
      newSelectionUnless(fave, p, mrg, add);
      startMoveSelection();
    }
  }
}

void EData::dropFromSelection(int id, Point p, Dim mrg) {
  // throw ID out of selection, then recollect SELPTS.
  selection.remove(id);
  selpts.clear();

  Object const &obj(layout.root().object(id));
  if (obj.isTrace()) {
    Trace const &t(obj.asTrace());
    if (t.onP1(p, mrg)) 
      purepts.remove(t.p1);
    if (t.onP2(p, mrg)) 
      purepts.remove(t.p2);
  }
  
  for (int k: selection)
    selectPointsOf(k);
  ed->update();
  ed->selectionChanged();
}

void EData::startMoveSelection() {
  moving = true;
  movingstart = presspoint.roundedTo(layout.board().grid);
  // If presspoint is on a hole or point, movingstart should actually be
  // shifted by distance of that point to the nearest grid intersection.
  movingdelta = Point();
}

void EData::newSelectionUnless(int id, Point p, Dim mrg, bool add) {
  // does not clear purepts if on a purept
  Object const &obj(layout.root().object(id));
  if (obj.isTrace()) {
    Trace const &t(obj.asTrace());
    if (t.onP1(p, mrg)) {
      if (purepts.contains(t.p1)) {
	if (add)
	  ed->deselectPoint(t.p1);
      } else {
	ed->selectPoint(t.p1, add);
      }
    } else if (t.onP2(p, mrg)) {
      if (purepts.contains(t.p2)) {
	if (add)
	  ed->deselectPoint(t.p2);
      } else {
	ed->selectPoint(t.p2, add);
      }
    } else {
      ed->select(id, add);
    }
  } else {
    ed->select(id, add);
  }
}


void EData::moveBanding(Point p) {
  if (!rubberband)
    return;
  rubberband->setGeometry(QRectF(mils2widget.map(presspoint.toMils()),
				 mils2widget.map(p.toMils())).normalized()
			  .toRect());
}

void EData::releaseMoving(Point p) {
  movingdelta = p.roundedTo(layout.board().grid) - movingstart;
  Group &here(layout.root().subgroup(crumbs));
  for (int id: here.keys()) {
    Object &obj(here.object(id));
    if (selection.contains(id)) {
      obj.translate(movingdelta);
    } else if (obj.isTrace()) {
      Trace &t = obj.asTrace();
      if (selpts.contains(t.p1) || purepts.contains(t.p1))
	t.p1 += movingdelta;
      if (selpts.contains(t.p2) || purepts.contains(t.p2))
	t.p2 += movingdelta;
    }
  }

  QSet<Point> newsel;
  for (Point const &pt: selpts)
    newsel << pt + movingdelta;
  selpts = newsel;
  QSet<Point> newpure;
  for (Point const &pt: purepts)
    newpure << pt + movingdelta;
  purepts = newpure;
  
  moving = false;
  ed->update();
}

void EData::releaseBanding(Point p) {
  delete rubberband;
  rubberband = 0;
  Rect r(presspoint, p);
  ed->selectArea(r, true);
}

Editor::Editor(QWidget *parent): QWidget(parent), d(new EData(this)) {
  setMouseTracking(true);
  setFocusPolicy(Qt::StrongFocus);
  scaleToFit();
}

Editor::~Editor() {
  delete d;
}

void Editor::setMode(Mode m) {
  d->mode = m;
}

bool Editor::load(QString fn) {
  d->layout = FileIO::loadLayout(fn);
  scaleToFit();
  update();
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
  d->mils2px = scale;
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

void Editor::mousePressEvent(QMouseEvent *e) {
  Point p = Point::fromMils(d->widget2mils.map(e->pos()));
  if (e->button() == Qt::LeftButton) {
    d->presspoint = p;
    switch (d->mode) {
    case Mode::Edit:
      d->pressEdit(p, e->modifiers());
      break;
    case Mode::PlaceTrace:
      d->pressTracing(p);
      break;
    default:
      break;
    }
  }
  QWidget::mousePressEvent(e);
}

void Editor::mouseMoveEvent(QMouseEvent *e) {
  Point p = Point::fromMils(d->widget2mils.map(e->pos()));
  if (d->tracing)
    d->moveTracing(p);
  else if (d->rubberband)
    d->moveBanding(p);
  else if (d->moving)
    d->moveMoving(p);
  emit hovering(p);
}

void Editor::mouseReleaseEvent(QMouseEvent *e) {
  Point p = Point::fromMils(d->widget2mils.map(e->pos()));
  if (d->rubberband)
    d->releaseBanding(p);
  else if (d->moving)
    d->releaseMoving(p);
}

void Editor::keyPressEvent(QKeyEvent *e) {
  switch (e->key()) {
  case Qt::Key_Escape:
    d->abortTracing();
    break;
  case Qt::Key_Enter: case Qt::Key_Return:
    if (d->tracing) {
      d->pressTracing(d->tracecurrent);
      d->abortTracing();
    }
    break;
  default:
    break;
  }
}

void Editor::enterEvent(QEvent *) {
}

void Editor::leaveEvent(QEvent *) {
  d->abortTracing();
  emit leaving();
}

void Editor::paintEvent(QPaintEvent *) {
  QPainter p(this);
  d->drawBoard(p);
  d->drawGrid(p);
  d->drawObjects(p);
  d->drawSelectedPoints(p);
  d->drawTracing(p);
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
  Group &here(d->layout.root().subgroup(d->crumbs));
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
  Group &here(d->layout.root().subgroup(d->crumbs));
  if (!add) {
    d->selection.clear();
    d->selpts.clear();
    d->purepts.clear();
  }
  if (here.contains(id)) {
    d->selection.insert(id);
    d->selectPointsOf(id);
  }
  update();
  emit selectionChanged();
}

void Editor::selectPoint(Point p, bool add) {
  if (!add) {
    d->selection.clear();
    d->selpts.clear();
    d->purepts.clear();
  }
  if (!d->selpts.contains(p))
    d->purepts.insert(p);
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
  if (d->selpts.contains(p) || d->purepts.contains(p)) {
    d->selpts.remove(p);
    d->purepts.remove(p);
    update();
    emit selectionChanged();
  }
}

void Editor::selectAll() {
  Group const &here(d->layout.root().subgroup(d->crumbs));
  d->selection = QSet<int>::fromList(here.keys());
  d->purepts.clear();
  for (int id: d->selection)
    d->selectPointsOf(here.object(id));
  update();
  emit selectionChanged();
}

void Editor::clearSelection() {
  d->selection.clear();
  d->selpts.clear();
  d->purepts.clear();
  update();
  emit selectionChanged();
}

void Editor::selectArea(Rect r, bool add) {
  if (!add) {
    d->selection.clear();
    d->selpts.clear();
    d->purepts.clear();
  }    
  Group const &here(d->layout.root().subgroup(d->crumbs));
  Point origin(d->layout.root().originOf(d->crumbs));
  r = r.translated(-origin);
  for (int id: here.keys()) {
    Object const &obj(here.object(id));
    if (!d->selection.contains(id)) {
      if (r.contains(obj.boundingRect())) {
	d->selection << id;
	d->selectPointsOf(obj);
      }
    }
  }
  for (int id: here.keys()) {
    Object const &obj(here.object(id));
    if (obj.type()==Object::Type::Trace) {
      Trace const &t(obj.asTrace());
      if (r.contains(t.p1) && !d->selpts.contains(t.p1))
	d->purepts << t.p1;
      if (r.contains(t.p2) && !d->selpts.contains(t.p2)) 
	d->purepts << t.p2;
    }
  }
  update();
  emit selectionChanged();
}

void Editor::setLineWidth(Dim l) {
  d->props.linewidth = l;
  // also affect current selection!
}

void Editor::setLayer(Layer l) {
  d->props.layer = l;
  // also affect current selection!
}
