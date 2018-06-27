// Editor.cpp

#include "Editor.h"
#include "data/FileIO.h"
#include "data/Layout.h"
#include "data/Orient.h"
#include "data/Object.h"
#include "ORenderer.h"

#include <QTransform>
#include <QPainter>
#include <QBrush>
#include <QPen>
#include <QRubberBand>
#include <QInputDialog>
#include <QMimeData>

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
  }
  void drawBoard(QPainter &) const;
  void drawGrid(QPainter &) const;
  void drawSelectedPoints(QPainter &) const;
  void drawObjects(QPainter &) const;
  void drawObject(Object const &o, Layer l, Point const &origin,
		  QPainter &p, bool selected, bool toplevel=false) const;
  // only draw parts of object that are part of given layer
  void drawTracing(QPainter &) const;
  void pressEdit(Point, Qt::KeyboardModifiers);
  int visibleObjectAt(Point p, Dim mrg=Dim()) const;
  void pressPad(Point);
  void pressArc(Point);
  void pressHole(Point);
  void pressText(Point);
  void pressTracing(Point);
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
  QSet<Point> pointsOf(Object const &obj, Point const &ori) const;
  Rect selectionBounds() const; // board coordinates
  void validateStuckPoints() const;
  void invalidateStuckPoints() const;
  void zoom(double factor);
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
  mutable bool stuckptsvalid; // indicator: if false, stuckpts needs to be
  // recalced.
  mutable QSet<Point> stuckpts; // points that are part of a selected object but
  // also of a non-trace non-selected object and that should not move
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
};

void EData::invalidateStuckPoints() const {
  stuckptsvalid = false;
}

void EData::validateStuckPoints() const {
  if (stuckptsvalid)
    return;
  // stuckpts will be the | of all points of nonselected nontraces, but only
  // if those points are also in selpts. (Others are irrelevant.)
  stuckpts.clear();
  Group const &here(layout.root().subgroup(crumbs));
  Point ori = layout.root().originOf(crumbs);
  for (int id: here.keys()) {
    if (!selection.contains(id)) {
      Object const &obj(here.object(id));
      if (!obj.isTrace())
        stuckpts |= pointsOf(obj, ori);
    }
  }
  stuckpts &= selpts;
  stuckptsvalid = true;
  qDebug() << "stuckpts:" << stuckpts;
}

Rect EData::selectionBounds() const {
  Group const &here(layout.root().subgroup(crumbs));
  Point ori = layout.root().originOf(crumbs);
  
  Rect r;
  for (int id: selection)
    r |= here.object(id).boundingRect();
  r = r.translated(ori);

  for (Point p: purepts)
    r |= p;

  return r;
}
  

void EData::selectPointsOf(int id) {
  Group const &here(layout.root().subgroup(crumbs));
  if (here.contains(id))
    selpts |= pointsOf(here.object(id), layout.root().originOf(crumbs));
}

QSet<Point> EData::pointsOf(Object const &obj, Point const &ori) const {
  QSet<Point> pp;
  switch (obj.type()) {
  case Object::Type::Null:
    break;
  case Object::Type::Hole:
    pp << obj.asHole().p + ori;
    break;
  case Object::Type::Pad:
    pp << obj.asPad().p + ori;
    break;
  case Object::Type::Arc:
    pp << obj.asArc().center + ori;
    break;
  case Object::Type::Text:
    break;
  case Object::Type::Trace:
    pp << obj.asTrace().p1 + ori << obj.asTrace().p2 + ori;
    break;
  case Object::Type::Group:
    for (Point const &p: obj.asGroup().points())
      pp << p + ori;
    break;
  case Object::Type::Plane:
    break;
  }
  return pp;
}

void EData::drawBoard(QPainter &p) const {
  p.setBrush(QBrush(QColor(0,0,0)));
  double lw = layout.board().width.toMils();
  double lh = layout.board().height.toMils();
  p.drawRect(QRectF(QPointF(0,0), QPointF(lw, lh)));
}

void EData::drawGrid(QPainter &p) const {
  // draw dots at either 0.1” or 2 mm intervals
  // and larger markers at either 0.5" or 10 mm intervals
  bool metric = layout.board().grid.isNull()
    ? layout.board().metric
    : layout.board().grid.isMetric();
  double lgrid = metric ? 2000/25.4 : 100;
  double wgdx = lgrid;
  double wgdy = lgrid;
  double wx0 = 0;
  double wx1 = layout.board().width.toMils();
  double wy0 = 0;
  double wy1 = layout.board().height.toMils();
  constexpr int major = 5;
  p.setPen(QPen(QColor(255, 255, 255), 1.0/mils2px));
  QPointF dpx(2,0);
  QPointF dpy(0,2);
  if (wgdy*mils2px >= 10 && wgdy*mils2px >= 10) {
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
  p.setPen(QPen(layerColor(props.layer), props.linewidth.toMils()*mils2px,
		Qt::SolidLine, Qt::RoundCap));
  p.drawLine(tracestart.toMils(), tracecurrent.toMils());
}

void EData::drawObjects(QPainter &p) const {
  Group const &here(layout.root().subgroup(crumbs));
  Point origin = layout.root().originOf(crumbs);
  validateStuckPoints();  

  ORenderer rndr(&p, origin);
  if (moving)
    rndr.setMoving(movingdelta);
  rndr.setSelPoints(selpts);
  rndr.setPurePoints(purepts);
  rndr.setStuckPoints(stuckpts);
  
  auto onelayer = [&](Layer l) {
    rndr.setLayer(l);
    for (int id: here.keys())
      rndr.drawObject(here.object(id), selection.contains(id));
  };

  auto drawplanes = [&](Layer) {
    // Bottom filled plane is easy.
    // Top filled plane is tricky because holes should let bottom shine through.
    // The full solution is to render the plane into a pixmap first, then
    // copy to the widget.
    // An alternative is to ADD the plane to the widget, then SUBTRACT
    // the holes, using appropriate color scheme. That would let bottom traces
    // shine through, but I am OK with that.
  };
  
  Board const &brd = layout.board();

  if (brd.layervisible[Layer::Bottom]) {
    if (brd.planesvisible)
      drawplanes(Layer::Bottom);
    onelayer(Layer::Bottom);
  }
  if (brd.layervisible[Layer::Top]) {
    if (brd.planesvisible)
      drawplanes(Layer::Top);
    onelayer(Layer::Top);
  }
  if (brd.layervisible[Layer::Silk])
    onelayer(Layer::Silk);
  onelayer(Layer::Invalid); // magic to punch holes
}

void EData::drawSelectedPoints(QPainter &p) const {
  if (purepts.isEmpty())
    return;
  p.setPen(QPen(Qt::NoPen));
  p.setBrush(QColor(200, 200, 200));
  for (Point pt: purepts) {
    if (moving)
      pt += movingdelta;
    p.drawEllipse(pt.toMils(), 25, 25);
  }
}

void EData::abortTracing() {
  tracing = false;
  ed->update();
}

void EData::pressText(Point p) {
  if (props.text.isEmpty())
    props.text = QInputDialog::getText(ed, "Place text", "Text:");
  if (props.text.isEmpty())
    return;

  p = p.roundedTo(layout.board().grid);
  Group &here(layout.root().subgroup(crumbs));
  p -= layout.root().originOf(crumbs);
  Text t;
  t.p = p;
  t.fontsize = props.fs;
  t.orient = props.orient;
  t.text = props.text;
  t.layer = props.layer;
  here.insert(Object(t));
  ed->update();
} 

void EData::pressHole(Point p) {
  p = p.roundedTo(layout.board().grid);
  Group &here(layout.root().subgroup(crumbs));
  p -= layout.root().originOf(crumbs);
  Hole t;
  t.p = p;
  t.od = props.od;
  t.id = props.id;
  t.square = props.square;
  here.insert(Object(t));
  ed->update();
}


void EData::pressPad(Point p) {
  p = p.roundedTo(layout.board().grid);
  Group &here(layout.root().subgroup(crumbs));
  p -= layout.root().originOf(crumbs);
  Pad t;
  t.p = p;
  t.width = props.w;
  t.height = props.h;
  here.insert(Object(t));
  ed->update();
}

void EData::pressArc(Point p) {
  p = p.roundedTo(layout.board().grid);
  Group &here(layout.root().subgroup(crumbs));
  p -= layout.root().originOf(crumbs);
  Arc t;
  t.center = p;
  t.radius = props.id / 2;
  t.linewidth = props.linewidth;
  t.extent = props.ext;
  here.insert(Object(t));
  ed->update();
}

void EData::pressTracing(Point p) {
  p = p.roundedTo(layout.board().grid);
  if (p == tracestart) {
    abortTracing();
    return;
  }
  if (tracing) {
    Group &here(layout.root().subgroup(crumbs));
    Point ori(layout.root().originOf(crumbs));
    Trace t;
    t.p1 = tracestart - ori;
    t.p2 = p - ori;
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
  Group const &group(layout.root().subgroup(crumbs));
  Point ori(layout.root().originOf(crumbs));
  p -= ori;
  QList<int> ids = group.objectsAt(p, mrg);
  /* Now, we want to select one item that P is on.
     We prioritize higher layers over lower layers, ignore pads, text, traces
     on hidden layers, prioritize holes, pads, groups [components], text over
     traces, which are prioritized over planes.
     If P is on an endpoint of a segment, we need to know about that as well.
  */
  int fave = -1;
  Prio prio = Prio::None;
  Board const &brd = layout.board();
  auto better = [&prio](Prio p1) { return int(p1) > int(prio); };
  for (int id: ids) {
    Prio p1 = Prio::None;
    Object const &obj = layout.root().object(id);
    Layer l = obj.layer();
    switch (obj.type()) {
    case Object::Type::Plane:
      if (brd.layervisible[l])
	p1 = l==Layer::Bottom ? Prio::BottomPlane : Prio::TopPlane;
    break;
    case Object::Type::Trace:
      if (brd.layervisible[l])
	p1 = l==Layer::Bottom ? Prio::BottomTrace
	  : l==Layer::Top ? Prio::TopTrace
	  : Prio::Silk;
      break;
    case Object::Type::Text: case Object::Type::Pad: case Object::Type::Arc:
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

  Object const &obj(layout.root().subgroup(crumbs).object(id));
  if (obj.isTrace()) {
    p -= layout.root().originOf(crumbs);
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
  Group const &here(layout.root().subgroup(crumbs));
  Object const &obj(here.object(id));
  if (obj.isTrace()) {
    Point ori = layout.root().originOf(crumbs);
    Trace const &t(obj.asTrace());
    if (t.onP1(p - ori, mrg)) {
      if (purepts.contains(t.p1 + ori)) {
	if (add)
	  ed->deselectPoint(t.p1 + ori);
      } else {
	ed->selectPoint(t.p1 + ori, add);
      }
    } else if (t.onP2(p - ori, mrg)) {
      if (purepts.contains(t.p2 + ori)) {
	if (add)
	  ed->deselectPoint(t.p2 + ori);
      } else {
	ed->selectPoint(t.p2 + ori, add);
      }
    } else {
      ed->select(id, add);
    }
  } else {
    ed->select(id, add);
    if (obj.isGroup())
      ed->select(obj.asGroup().refTextId(), true);
  }
}


void EData::moveBanding(Point p) {
  if (!rubberband)
    return;
  rubberband->setGeometry(QRectF(mils2widget.map(presspoint.toMils()),
				 mils2widget.map(p.toMils())).normalized()
			  .toRect());
}

void EData::pressPanning(QPoint p) {
  autofit = false;
  pan0 = mils2widget;
  panstart = p;
  panning = true;
}

void EData::movePanning(QPoint p) {
  QPoint delta = p - panstart;
  mils2widget = pan0;
  mils2widget.translate(delta.x()/mils2px, delta.y()/mils2px);
  widget2mils = mils2widget.inverted();
  ed->update();
}

void EData::releaseMoving(Point p) {
  validateStuckPoints();
  movingdelta = p.roundedTo(layout.board().grid) - movingstart;
  Group &here(layout.root().subgroup(crumbs));
  Point ori(layout.root().originOf(crumbs));
  for (int id: here.keys()) {
    Object &obj(here.object(id));
    if (selection.contains(id)) {
      if (obj.isTrace()) {
        Trace &t = obj.asTrace();
        if (!stuckpts.contains(t.p1+ori))
          t.p1 += movingdelta;
        if (!stuckpts.contains(t.p2+ori))
          t.p2 += movingdelta;
      } else {
        obj.translate(movingdelta);
      }
    } else if (obj.isTrace()) {
      Trace &t = obj.asTrace();
      if ((selpts.contains(t.p1 + ori) || purepts.contains(t.p1 + ori))
          && !stuckpts.contains(t.p1+ori))
	t.p1 += movingdelta;
      if ((selpts.contains(t.p2 + ori) || purepts.contains(t.p2 + ori))
          && !stuckpts.contains(t.p2+ori))
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
  setAcceptDrops(true);
  setFocusPolicy(Qt::StrongFocus);
  scaleToFit();
}

Editor::~Editor() {
  delete d;
}

void Editor::setMode(Mode m) {
  d->mode = m;
  clearSelection();
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
  d->zoom(sqrt(2));
}

void Editor::zoomOut() {
  d->zoom(1/sqrt(2));
}

void EData::zoom(double factor) {
  autofit = false;
  QPointF xy0m = hoverpt.toMils();
  QPointF xy0 = mils2widget.map(xy0m); // where do we project now?
  mils2widget.scale(factor, factor);
  QPointF xy1m = mils2widget.inverted().map(xy0);
  mils2widget.translate(xy1m.x()-xy0m.x(), xy1m.y()-xy0m.y());
  mils2px = mils2widget.m11(); // *= factor;
  widget2mils = mils2widget.inverted();
  ed->update();
}

void Editor::wheelEvent(QWheelEvent *e) {
  d->zoom(pow(2, e->angleDelta().y()/240.));
}

void Editor::resizeEvent(QResizeEvent *) {
  if (d->autofit)
    scaleToFit();
}

void Editor::mousePressEvent(QMouseEvent *e) {
  Point p = Point::fromMils(d->widget2mils.map(e->pos()));
  if (e->button() == Qt::LeftButton) {
    d->presspoint = p;
    if (e->modifiers() & Qt::ControlModifier) {
      d->pressPanning(e->pos());
    } else {
      d->panning = false;
      switch (d->mode) {
      case Mode::Edit:
	d->pressEdit(p, e->modifiers());
	break;
      case Mode::PlaceTrace:
	d->pressTracing(p);
	break;
      case Mode::PlaceHole:
	d->pressHole(p);
	break;
      case Mode::PlaceText:
	d->pressText(p);
	break;
      case Mode::PlacePad:
	d->pressPad(p);
	break;
      case Mode::PlaceArc:
	d->pressArc(p);
	break;
      default:
	break;
      }
    }
  }
  QWidget::mousePressEvent(e);
}

void Editor::mouseMoveEvent(QMouseEvent *e) {
  Point p = Point::fromMils(d->widget2mils.map(e->pos()));
  if (d->panning)
    d->movePanning(e->pos());
  else if (d->tracing)
    d->moveTracing(p);
  else if (d->rubberband)
    d->moveBanding(p);
  else if (d->moving)
    d->moveMoving(p);
  d->hoverpt = p;
  emit hovering(p);
}

void Editor::mouseReleaseEvent(QMouseEvent *e) {
  Point p = Point::fromMils(d->widget2mils.map(e->pos()));
  if (d->rubberband)
    d->releaseBanding(p);
  else if (d->moving)
    d->releaseMoving(p);
  d->panning = false;
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
  d->hoverpt
    = Point::fromMils(d->widget2mils.map(QPoint(width()/2, height()/2)));
  emit leaving();
}

void Editor::paintEvent(QPaintEvent *) {
  QPainter p(this);
  p.setTransform(d->mils2widget, true);
  d->drawBoard(p);
  d->drawGrid(p);
  d->drawObjects(p);
  d->drawSelectedPoints(p);
  d->drawTracing(p);
}

void Editor::setGrid(Dim g) {
  Board &brd = d->layout.board();  
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
  d->invalidateStuckPoints();
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
  d->invalidateStuckPoints();
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
  d->invalidateStuckPoints();
  if (d->selection.contains(id)) {
    d->selection.remove(id);
    update();
    emit selectionChanged();
  }
}

void Editor::deselectPoint(Point p) {
  d->invalidateStuckPoints();
  if (d->selpts.contains(p) || d->purepts.contains(p)) {
    d->selpts.remove(p);
    d->purepts.remove(p);
    update();
    emit selectionChanged();
  }
}

void Editor::selectAll() {
  d->invalidateStuckPoints();
  Group const &here(d->layout.root().subgroup(d->crumbs));
  d->selection = QSet<int>::fromList(here.keys());
  d->purepts.clear();
  for (int id: d->selection)
    d->selectPointsOf(id);
  update();
  emit selectionChanged();
}

void Editor::clearSelection() {
  d->invalidateStuckPoints();
  d->selection.clear();
  d->selpts.clear();
  d->purepts.clear();
  update();
  emit selectionChanged();
}

void Editor::selectArea(Rect r, bool add) {
  d->invalidateStuckPoints();
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
        if (obj.isText() && obj.asText().groupAffiliation()>0) {
          // don't rectangle select a group reference text
        } else if (obj.isGroup()) {
          d->selection << id;
          d->selectPointsOf(id);
          int tid = obj.asGroup().refTextId();
          if (here.contains(tid))
            d->selection << tid;
        } else {
          d->selection << id;
          d->selectPointsOf(id);
        }          
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

void Editor::setExtent(Arc::Extent ext) {
  d->props.ext = ext;
  Group &here(d->layout.root().subgroup(d->crumbs));
  for (int id: d->selection) {
    Object &obj(here.object(id));
    if (obj.isArc())
      obj.asArc().extent = ext;
  }
  update();
}

void Editor::setLineWidth(Dim l) {
  d->props.linewidth = l;
  Group &here(d->layout.root().subgroup(d->crumbs));
  for (int id: d->selection) {
    Object &obj(here.object(id));
    if (obj.isTrace())
      obj.asTrace().width = l;
    else if (obj.isArc())
      obj.asArc().linewidth = l;
  }
  update();
}

void Editor::setLayer(Layer l) {
  d->props.layer = l;

  Group &here(d->layout.root().subgroup(d->crumbs));
  for (int id: d->selection) {
    Object &obj(here.object(id));
    switch (obj.type()) {
    case Object::Type::Trace:
      obj.asTrace().layer = l;
      break;
    case Object::Type::Text:
      obj.asText().setLayer(l);
      break;
    case Object::Type::Pad:
      obj.asPad().layer = l;
      break;
    case Object::Type::Arc:
      obj.asArc().layer = l;
      break;
    default:
      break;
    }
  }
  update();
}

void Editor::setID(Dim x) {
  if (x < Dim::fromInch(.005))
    x = Dim::fromInch(.005);
  d->props.id = x;

  Group &here(d->layout.root().subgroup(d->crumbs));
  for (int id: d->selection) {
    Object &obj(here.object(id));
    if (obj.isHole()) {
      obj.asHole().id = x;
      if (obj.asHole().od < x + Dim::fromInch(.015))
	obj.asHole().od = x + Dim::fromInch(.015);
    } else if (obj.isArc()) {
      obj.asArc().radius = x/2;
    }
  }
  update();
}

void Editor::setOD(Dim x) {
  if (x < Dim::fromInch(.02))
    x = Dim::fromInch(.02);
  d->props.od = x;

  Group &here(d->layout.root().subgroup(d->crumbs));
  for (int id: d->selection) {
    Object &obj(here.object(id));
    if (obj.type()==Object::Type::Hole) {
      obj.asHole().od = x;
      if (obj.asHole().id > x - Dim::fromInch(.015))
	obj.asHole().id = x - Dim::fromInch(.015);
    }      
  }
  update();
}

void Editor::setWidth(Dim x) {
  if (x < Dim::fromInch(.01))
    x = Dim::fromInch(.01);
  d->props.w = x;

  Group &here(d->layout.root().subgroup(d->crumbs));
  for (int id: d->selection) {
    Object &obj(here.object(id));
    if (obj.type()==Object::Type::Pad)
      obj.asPad().width = x;
  }
  update();
}

void Editor::setHeight(Dim x) {
  if (x < Dim::fromInch(.01))
    x = Dim::fromInch(.01);
  d->props.h = x;

  Group &here(d->layout.root().subgroup(d->crumbs));
  for (int id: d->selection) {
    Object &obj(here.object(id));
    if (obj.type()==Object::Type::Pad)
      obj.asPad().height = x;
  }
  update();
}

void Editor::setSquare(bool b) {
  d->props.square = b;

  Group &here(d->layout.root().subgroup(d->crumbs));
  for (int id: d->selection) {
    Object &obj(here.object(id));
    if (obj.type()==Object::Type::Hole)
      obj.asHole().square = b;
  }
  update();
}

void Editor::setRef(QString t) {
  Group &here(d->layout.root().subgroup(d->crumbs));
  for (int id: d->selection) {
    Object &obj(here.object(id));
    if (obj.isHole()) {
      obj.asHole().ref = t;
    } else if (obj.isGroup()) {
      Group &g(obj.asGroup());
      g.ref = t;
      int tid = g.refTextId();
      if (tid>0 && here.contains(tid))
        here.object(tid).asText().text = t;
    }
  }
  update();
}

void Editor::setText(QString t) {
  d->props.text = t;
  Group &here(d->layout.root().subgroup(d->crumbs));
  for (int id: d->selection) {
    Object &obj(here.object(id));
    if (obj.isText()) {
      Text &txt(obj.asText());
      txt.text = t;
      int gid = txt.groupAffiliation();
      if (gid>0 && here.contains(gid))
        here.object(gid).asGroup().ref = t;
    }
  }
  update();
}

void Editor::setRotation(int rot) {
  d->props.orient.rot = rot;
}

void Editor::setFlipped(bool f) {
  d->props.orient.flip = f;
}

void Editor::setFontSize(Dim fs) {
  d->props.fs = fs;
  Group &here(d->layout.root().subgroup(d->crumbs));
  for (int id: d->selection) {
    Object &obj(here.object(id));
    if (obj.isText())
      obj.asText().fontsize = fs;
  }
  update();
}

QList<int> Editor::breadcrumbs() const {
  return d->crumbs;
}

QSet<int> Editor::selectedObjects() const {
  return d->selection;
}

QSet<Point> Editor::selectedPoints() const {
  return d->purepts | d->selpts;
}

Group const &Editor::currentGroup() const {
  return d->layout.root().subgroup(d->crumbs);
}

Point Editor::groupOffset() const {
  return d->layout.root().originOf(d->crumbs);
}

void Editor::rotateCW() {
  Group &here(d->layout.root().subgroup(d->crumbs));
  Point origin(d->layout.root().originOf(d->crumbs));
  Rect box(d->selectionBounds());
  if (box.isEmpty())
    return;
  Point center = box.center(); //.roundedTo(d->layout.board().grid);
  center -= origin;
  
  for (int id: d->selection)
    here.object(id).rotateCW(center);

  Dim mrg = Dim::fromMils(4/d->mils2px);
  for (Point p: d->purepts | d->selpts) {
    // Rotate end points of traces if those end points are in purepts,
    // but the trace itself is not in selection.
    for (int id: here.keys()) {
      if (!d->selection.contains(id)) {
	Object &obj(here.object(id));
	if (obj.isTrace()) {
	  Trace &t(obj.asTrace());
	  if (t.onP1(p - origin, mrg))
	    t.p1.rotateCW(center);
	  if (t.onP2(p - origin, mrg))
	    t.p2.rotateCW(center);
	}
      }
    }
  }

  QSet<Point> newpts;
  for (Point const &p: d->purepts)
    newpts << p.rotatedCW(center);
  d->purepts = newpts;
  newpts.clear();
  for (Point const &p: d->selpts)
    newpts << p.rotatedCW(center);
  d->selpts = newpts;
  
  update();
}

void Editor::rotateCCW() {
  rotateCW();
  rotateCW();
  rotateCW();
}

void Editor::flipH() {
  Group &here(d->layout.root().subgroup(d->crumbs));
  Point origin(d->layout.root().originOf(d->crumbs));
  Rect box(d->selectionBounds());
  if (box.isEmpty())
    return;
  Dim center = box.center().x; //.roundedTo(d->layout.board().grid).x;
  center -= origin.x;
  
  for (int id: d->selection)
    here.object(id).flipLeftRight(center);

  Dim mrg = Dim::fromMils(4/d->mils2px);
  for (Point p: d->purepts | d->selpts) {
    // Rotate end points of traces if those end points are in purepts,
    // but the trace itself is not in selection.
    for (int id: here.keys()) {
      if (!d->selection.contains(id)) {
	Object &obj(here.object(id));
	if (obj.isTrace()) {
	  Trace &t(obj.asTrace());
	  if (t.onP1(p - origin, mrg))
	    t.p1.flipLeftRight(center);
	  if (t.onP2(p - origin, mrg))
	    t.p2.flipLeftRight(center);
	}
      }
    }
  }
  QSet<Point> newpts;
  for (Point const &p: d->purepts)
    newpts << p.flippedLeftRight(center);
  d->purepts = newpts;
  newpts.clear();
  for (Point const &p: d->selpts)
    newpts << p.flippedLeftRight(center);
  d->selpts = newpts;

  update();
}

void Editor::flipV() {
  rotateCW();
  flipH();
  rotateCCW();
}

EProps &Editor::properties() {
  return d->props;
}

void Editor::formGroup() {
  if (d->selection.isEmpty())
    return;
  Group &here(d->layout.root().subgroup(d->crumbs));
  here.formSubgroup(d->selection);
  clearSelection();
  update();
}

void Editor::dissolveGroup() {
  if (d->selection.isEmpty())
    return;
  Group &here(d->layout.root().subgroup(d->crumbs));
  for (int id: d->selection) 
    if (here.object(id).isGroup())
      here.dissolveSubgroup(id);
  clearSelection();
  update();
}

void Editor::deleteSelected() {
  if (d->selection.isEmpty())
    return;
  Group &here(d->layout.root().subgroup(d->crumbs));
  for (int id: d->selection) {
    if (here.object(id).isGroup()) {
      int tid = here.object(id).asGroup().refTextId();
      if (tid>0)
        here.remove(tid);
      here.remove(id);
    } else if (here.object(id).isText()
               && here.object(id).asText().groupAffiliation()>0) {
      // refuse to delete ref text object
    } else {
      here.remove(id);
    }
  }
  clearSelection();
  update();
}
  
int Editor::selectedComponent(QString *msg) const {
  Group &here(d->layout.root().subgroup(d->crumbs));
  int gid = 0;
  int tid = 0;
  QString m = "Nothing selected";
  for (int id: d->selection) {
    Object const &obj(here.object(id));
    if (obj.isGroup()) {
      qDebug() << "found group" << id << "already had" << gid << tid;
      if (gid && gid!=id) {
        m = "More than one group selected";
        gid = 0;
        break;
      } else {
        gid = id;
        int tid1 = obj.asGroup().refTextId();
	qDebug() << "found group" << id << "("<<tid1
		 <<") already had" << gid << tid;
        if (tid && tid!=tid1) {
          m = "More than one group selected";
          gid = 0;
          break;
        } else {
          tid = tid1;
        }
      }
    } else if (obj.isText()) {
      int gid1 = obj.asText().groupAffiliation();
      qDebug() << "found text" << id << "("<<gid1
	       <<") already had" << gid << tid;
      if (gid1==0 || (gid && gid1!=gid)) {
        m = "More than one item selected";
        gid = 0;
        break;
      } else {
        tid = id;
        gid = gid1;
      }
    } else {
      qDebug() << "found other object" << id << int(obj.type())
	       << "already had" << gid << tid;
      if (d->selection.size()>1)
	m = "More than one item selected";
      else
	m = "Nongroup selected";
      break;
    }
  }
  if (msg)
    *msg = m;
  return gid;
}

bool Editor::saveComponent(int id, QString fn) const {
  return d->layout.root().subgroup(d->crumbs).saveComponent(id, fn);
}

bool Editor::insertComponent(QString fn, Point pt) {
  Group &here(d->layout.root().subgroup(d->crumbs));
  Point ori = d->layout.root().originOf(d->crumbs);
  int gid = here.insertComponent(fn);
  if (!gid)
    return false;
  Object &obj(here.object(gid));
  Rect bb = obj.boundingRect();
  Point delta = pt - ori - bb.center();
  obj.translate(delta);
  int tid = obj.asGroup().refTextId();
  if (here.contains(tid))
    here.object(tid).translate(delta);
  select(gid, false);
  select(tid, true);
  return true;
}

Point Editor::hoverPoint() const {
  return d->hoverpt;
}

void Editor::dragEnterEvent(QDragEnterEvent *e) {
  QMimeData const *md = e->mimeData();
  if (md->hasUrls()) {
    QList<QUrl> urls = md->urls();
    QString fn;
    for (QUrl url: urls) {
      if (url.isLocalFile() && url.path().endsWith(".svg")) {
	fn = url.path();
	break;
      }
    }
    if (!fn.isEmpty()) {
      e->accept();
    } else {
      e->ignore();
    }
  } else {
    e->ignore();
  }
}

void Editor::dragLeaveEvent(QDragLeaveEvent *e) {
  e->accept();
}

void Editor::dragMoveEvent(QDragMoveEvent *e) {
  e->accept();
}

void Editor::dropEvent(QDropEvent *e) {
  QMimeData const *md = e->mimeData();
  if (md->hasUrls()) {
    QList<QUrl> urls = md->urls();
    bool take = false;
    clearSelection();
    for (QUrl url: urls)
      if (url.isLocalFile() && url.path().endsWith(".svg"))
        take = insertComponent(url.path(),
			       Point::fromMils(d->widget2mils.map(e->pos())));
    if (take) {
      update();
      e->accept();
    } else {
      e->ignore();
    }
  } else {
    e->ignore();
  }  
}
