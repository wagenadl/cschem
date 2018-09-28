// Editor.cpp

#include "Editor.h"
#include "EData.h"
#include "UndoCreator.h"
#include "Tracer.h"
#include "PlaneEditor.h"
#include "data/TraceRepair.h"

#include <QInputDialog>
#include <QResizeEvent>
#include <QTimer>

Editor::Editor(QWidget *parent): QWidget(parent), d(new EData(this)) {
  setMouseTracking(true);
  setAcceptDrops(true);
  setFocusPolicy(Qt::StrongFocus);
  scaleToFit();
}

Editor::~Editor() {
  delete d;
}

void Editor::setAngleConstraint(bool c) {
  d->props.angleconstraint = c;
}

void Editor::setMode(Mode m) {
  d->mode = m;
  d->abortTracing();
  clearSelection();
  if (m==Mode::PlacePlane) {
    if (!d->planeeditor)
      d->planeeditor = new PlaneEditor(d);
  } else {
    if (d->planeeditor) {
      delete d->planeeditor;
      d->planeeditor = 0;
    }
  }
  update();
}

bool Editor::load(QString fn) {
  d->layout = FileIO::loadLayout(fn);
  if (d->layout.root().isEmpty())
    d->layout = Layout();
  d->stepsfromsaved = 0;
  d->undostack.clear();
  d->redostack.clear();
  emit undoAvailable(false);
  emit redoAvailable(false);
  emit changedFromSaved(false);
  scaleToFit();
  update();
  d->linkedschematic.link(d->layout.board().linkedschematic);
  return !d->layout.root().isEmpty();
}

bool Editor::save(QString fn) {
  if (FileIO::saveLayout(fn, d->layout)) {
    d->stepsfromsaved = 0;
    emit changedFromSaved(false);
    return true;
  } else {
    return false;
  }
}

Layout const &Editor::pcbLayout() const {
  return d->layout;
}

double Editor::pixelsPerMil() const {
  return d->mils2px;
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
  emit scaleChanged();
}

void Editor::zoomIn() {
  d->zoom(sqrt(2));
}

void Editor::zoomOut() {
  d->zoom(1/sqrt(2));
}

void Editor::wheelEvent(QWheelEvent *e) {
  d->zoom(pow(2, e->angleDelta().y()/240.));
}

void Editor::resizeEvent(QResizeEvent *e) {
  qDebug() << "editor::resizeevent" << e->oldSize() << e->size();
  if (!d->autofit)
    return;
  if (!d->resizeTimer) {
    d->resizeTimer = new QTimer(this);
    d->resizeTimer->setInterval(10);
    d->resizeTimer->setSingleShot(true);
    connect(d->resizeTimer, &QTimer::timeout,
	    [this]() { d->perhapsRefit(); });
  }
  d->resizeTimer->start();
}

void Editor::mouseDoubleClickEvent(QMouseEvent *e) {
  Point p = Point::fromMils(d->widget2mils.map(e->pos()));
  if (e->button() == Qt::LeftButton) {
    d->presspoint = p;
    if (d->mode == Mode::Edit) {
      Dim mrg = d->pressMargin();
      int fave = d->visibleObjectAt(p, mrg);
      if (fave<0) 
	leaveGroup();
      else 
	doubleClickOn(p, fave);
    } else if (d->planeeditor) {
      d->planeeditor->doubleClick(p, e->button(), e->modifiers());
    }
  }
}

void Editor::mousePressEvent(QMouseEvent *e) {
  Point p = Point::fromMils(d->widget2mils.map(e->pos()));
  if (e->button() == Qt::LeftButton) {
    d->presspoint = p;
    d->significantmove = false;
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
      case Mode::PickupTrace:
	d->pressPickingUp(p);
	break;
      case Mode::PlacePlane:
        Q_ASSERT(d->planeeditor);
        d->planeeditor->mousePress(p, e->button(), e->modifiers());
        break;
      default:
	break;
      }
    }
  }
  QWidget::mousePressEvent(e);
  updateOnNet();
}

void Editor::mouseMoveEvent(QMouseEvent *e) {
  Point p = Point::fromMils(d->widget2mils.map(e->pos()));

  if (d->panning)
    d->movePanning(e->pos());
  else if (d->tracer)
    d->moveTracing(p);
  else if (d->rubberband)
    d->moveBanding(p);
  else if (d->moving)
    d->moveMoving(p);

  d->hoverpt = p;
  emit hovering(p);

  if (!d->moving && !d->tracer)
    updateOnNet();
  if (d->planeeditor)
    d->planeeditor->mouseMove(p, e->button(), e->modifiers());
}

void Editor::updateOnNet() {
  if (d->updateOnWhat()) {
    emit onObject(d->onobject);
    QStringList mis;
    for (Nodename const &n: d->netmismatch.missingEntirely)
      mis << n.humanName();
    mis.sort();
    emit missingNodes(mis);
    if (d->netsvisible)
      update();
  }
}

void Editor::mouseReleaseEvent(QMouseEvent *e) {
  Point p = Point::fromMils(d->widget2mils.map(e->pos()));
  if (d->rubberband)
    d->releaseBanding(p);
  else if (d->moving)
    d->releaseMoving(p);
  if (d->planeeditor)
    d->planeeditor->mouseRelease(p, e->button(), e->modifiers());
  d->panning = false;
}

void Editor::keyPressEvent(QKeyEvent *e) {
  switch (e->key()) {
  case Qt::Key_Escape:
    if (d->tracer) {
      d->abortTracing();
    } else {
      if (e->modifiers() & Qt::ControlModifier)
	leaveAllGroups();
      else
	leaveGroup();
    }
    break;
  case Qt::Key_Enter: case Qt::Key_Return:
    if (d->tracer) {
      d->tracer->confirm();
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
  if (d->tracer)
    d->tracer->render(p);
  if (d->planeeditor)
    d->planeeditor->render(p);
}

void Editor::setGrid(Dim g) {
  Board &brd = d->layout.board();  
  if (g != brd.grid) {
    brd.grid = g;
    if (d->planeeditor)
      d->planeeditor->resetMouseMargin();
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

void Editor::setNetsVisibility(bool b) {
  d->netsvisible = b;
  d->updateOnWhat(true);
  update();
}

void Editor::setPlanesVisibility(bool b) {
  Board &brd = d->layout.board();  
  if (b != brd.planesvisible) {
    brd.planesvisible = b;
    update();
    emit boardChanged(brd);
  }
}

void Editor::doubleClickOn(Point p, int id) {
  Dim mrg = d->pressMargin();
  Group const &here(d->currentGroup());
  Object const &obj(here.object(id));
  switch (obj.type()) {
  case Object::Type::Group: {
    Group const &group(obj.asGroup());
    int fave = d->visibleObjectAt(group, p, mrg);
    if (fave>0 && (group.object(fave).isHole() || group.object(fave).isPad())) {
      d->editPinName(id, fave);
    } else {
      enterGroup(id);
      if (fave>=0)
	select(fave);
    }
  } break;
  case Object::Type::Hole: {
    bool ok;
    QString ref = QInputDialog::getText(this, "Hole properties",
                                        "Pin name/number:",
					QLineEdit::Normal,
					obj.asHole().ref, &ok);
    if (ok) {
      select(id);
      setRefText(ref);
      select(id);
      update();
    }
  } break;
  case Object::Type::Pad: {
    bool ok;
    QString ref = QInputDialog::getText(this, "Pad properties",
                                        "Pin name/number:",
					QLineEdit::Normal,
					obj.asPad().ref, &ok);
    if (ok) {
      select(id);
      setRefText(ref);
      select(id);
      update();
    }
  } break;
  case Object::Type::Text: {
    bool ok;
    bool isref = obj.asText().groupAffiliation()>0;
    QString what = isref ? "Group" : "Text";
    QString lbl = isref ? "Ref." : "Text";
    QString ref = QInputDialog::getText(this, what + " properties",
                                        lbl + ":",
					QLineEdit::Normal,
					obj.asText().text, &ok);
    if (ok) {
      select(id);
      setRefText(ref);
      select(id);
      update();
    }
  } break;
  default:
    break;
  }
}

bool Editor::enterGroup(int sub) {
  clearSelection();
  Group const &here(d->currentGroup());
  if (here.contains(sub) && here.object(sub).isGroup()) {
    d->crumbs << sub;
    d->updateOnWhat(true);
    update();
    return true;
  } else {
    return false;
  }
}

bool Editor::leaveGroup() {
  clearSelection();
  if (d->crumbs.isEmpty())
    return false;
  d->crumbs.takeLast();
  d->updateOnWhat(true);
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
  Group &here(d->currentGroup());
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
  d->emitSelectionStatus();
}

void Editor::selectPoint(Point p, bool add) {
  d->invalidateStuckPoints();
  if (!add) {
    d->selection.clear();
    d->selpts.clear();
    d->purepts.clear();
  }
  for (Layer l: ::layers()) 
    if (d->layout.board().layervisible[l])
      if (!d->selpts[l].contains(p))
	d->purepts[l].insert(p);
  update();
  d->emitSelectionStatus();
}

void Editor::deselect(int id) {
  d->invalidateStuckPoints();
  if (d->selection.contains(id)) {
    d->selection.remove(id);
    update();
    d->emitSelectionStatus();
  }
}

void Editor::deselectPoint(Point p) {
  d->invalidateStuckPoints();
  bool any = false;
  for (Layer l: layers()) {
    if (d->selpts[l].contains(p) || d->purepts[l].contains(p)) {
      d->selpts[l].remove(p);
      d->purepts[l].remove(p);
      any = true;
    }
  }
  if (any) {
    update();
    d->emitSelectionStatus();
  }
}

void Editor::selectAll() {
  d->invalidateStuckPoints();
  Group const &here(d->currentGroup());
  d->selection = QSet<int>::fromList(here.keys());
  d->purepts.clear();
  for (int id: d->selection)
    d->selectPointsOf(id);
  update();
  d->emitSelectionStatus();
}

void Editor::clearSelection() {
  d->invalidateStuckPoints();
  d->selection.clear();
  d->selpts.clear();
  d->purepts.clear();
  update();
  d->emitSelectionStatus();
}

void Editor::selectArea(Rect r, bool add) {
  d->invalidateStuckPoints();
  if (!add) {
    d->selection.clear();
    d->selpts.clear();
    d->purepts.clear();
  }    
  Group const &here(d->currentGroup());
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
      if (r.contains(t.p1) && !d->selpts[t.layer].contains(t.p1))
	d->purepts[t.layer] << t.p1;
      if (r.contains(t.p2) && !d->selpts[t.layer].contains(t.p2)) 
	d->purepts[t.layer] << t.p2;
    }
  }
  update();
  d->emitSelectionStatus();
}

void Editor::setArcAngle(int angle) {
  d->props.arcangle = angle;
  Group &here(d->currentGroup());
  UndoCreator uc(d);
  for (int id: d->selection) {
    Object &obj(here.object(id));
    if (obj.isArc()) {
      uc.realize();
      obj.asArc().angle = angle;
    }
  }
}

void Editor::setLineWidth(Dim l) {
  d->props.linewidth = l;
  Group &here(d->currentGroup());
  UndoCreator uc(d);
  for (int id: d->selection) {
    Object &obj(here.object(id));
    if (obj.isTrace()) {
      uc.realize();
      obj.asTrace().width = l;
    } else if (obj.isArc()) {
      uc.realize();
      obj.asArc().linewidth = l;
    }
  }
}

void Editor::setLayer(Layer l) {
  d->props.layer = l;

  Group &here(d->currentGroup());
  UndoCreator uc(d);
  for (int id: d->selection) {
    Object &obj(here.object(id));
    switch (obj.type()) {
    case Object::Type::Trace:
      uc.realize();
      obj.asTrace().layer = l;
      break;
    case Object::Type::Text:
      uc.realize();
      obj.asText().setLayer(l);
      break;
    case Object::Type::Pad:
      uc.realize();
      obj.asPad().layer = l;
      break;
    case Object::Type::Arc:
      uc.realize();
      obj.asArc().layer = l;
      break;
    default:
      break;
    }
  }
  if (d->planeeditor)
    update();
}

void Editor::setID(Dim x) {
  if (x < Dim::fromInch(.005))
    x = Dim::fromInch(.005);
  d->props.id = x;

  Group &here(d->currentGroup());
  UndoCreator uc(d);
  for (int id: d->selection) {
    Object &obj(here.object(id));
    if (obj.isHole()) {
      uc.realize();
      obj.asHole().id = x;
      if (obj.asHole().od < x + Dim::fromInch(.015))
	obj.asHole().od = x + Dim::fromInch(.015);
    } else if (obj.isArc()) {
      uc.realize();
      obj.asArc().radius = x/2;
    }
  }
}

void Editor::setOD(Dim x) {
  if (x < Dim::fromInch(.02))
    x = Dim::fromInch(.02);
  d->props.od = x;
  UndoCreator uc(d);

  Group &here(d->currentGroup());
  for (int id: d->selection) {
    Object &obj(here.object(id));
    if (obj.type()==Object::Type::Hole) {
      uc.realize();
      obj.asHole().od = x;
      if (obj.asHole().id > x - Dim::fromInch(.015))
	obj.asHole().id = x - Dim::fromInch(.015);
    }      
  }
}

void Editor::setWidth(Dim x) {
  if (x < Dim::fromInch(.01))
    x = Dim::fromInch(.01);
  d->props.w = x;

  Group &here(d->currentGroup());
  UndoCreator uc(d);
  for (int id: d->selection) {
    Object &obj(here.object(id));
    if (obj.type()==Object::Type::Pad) {
      uc.realize();
      obj.asPad().width = x;
    }
  }
}

void Editor::setHeight(Dim x) {
  if (x < Dim::fromInch(.01))
    x = Dim::fromInch(.01);
  d->props.h = x;
  UndoCreator uc(d);
  Group &here(d->currentGroup());
  for (int id: d->selection) {
    Object &obj(here.object(id));
    if (obj.type()==Object::Type::Pad) {
      uc.realize();
      obj.asPad().height = x;
    }
  }
}

void Editor::setSquare(bool b) {
  d->props.square = b;
  UndoCreator uc(d);
  Group &here(d->currentGroup());
  for (int id: d->selection) {
    Object &obj(here.object(id));
    if (obj.type()==Object::Type::Hole) {
      uc.realize();
      obj.asHole().square = b;
    }
  }
}

void Editor::setRefText(QString t) {
  d->props.text = t;
  Group const &here(d->currentGroup());
  bool cchg = false;
  UndoCreator uc(d);
  for (int id: d->selection) {
    Object const &obj(here.object(id));
    if (obj.isHole()) {
      uc.realize();
      d->currentGroup().object(id).asHole().ref = t;
    } else if (obj.isPad()) {
      uc.realize();
      d->currentGroup().object(id).asPad().ref = t;
    } else if (obj.isGroup()) {
      uc.realize();
      Group &here(d->currentGroup()); // this is needed, because any reference
      // taken *before* uc.realize() can inappropriately affect the undo copy
      Group &g(here.object(id).asGroup());
      g.ref = t;
      int tid = here.ensureRefText(id);
      if (tid>0 && here.contains(tid)) {
        here.object(tid).asText().text = t;
      }
      cchg = true;
    } else if (obj.isText()) {
      uc.realize();
      int gid;
      { Text &txt(d->currentGroup().object(id).asText());
        gid = txt.groupAffiliation();
        txt.text = t;
      } /* make sure reference goes out of scope before it becomes toxic
           by removal from the group */
      if (t.isEmpty())
        d->currentGroup().remove(id);
      if (gid>0 && here.contains(gid)) {
        d->currentGroup().object(gid).asGroup().ref = t;
        cchg = true;
      }
    }
  }
  if (cchg)
    emit componentsChanged();
}

void Editor::setRotation(int rot) {
  d->props.orient.rot = rot;
}

void Editor::setFlipped(bool f) {
  d->props.orient.flip = f;
}

void Editor::setFontSize(Dim fs) {
  d->props.fs = fs;
  Group &here(d->currentGroup());
  UndoCreator uc(d);
  for (int id: d->selection) {
    Object &obj(here.object(id));
    if (obj.isText()) {
      uc.realize();
      obj.asText().fontsize = fs;
    }
  }
}

NodeID Editor::breadcrumbs() const {
  return d->crumbs;
}

QSet<int> Editor::selectedObjects() const {
  return d->selection;
}

QSet<Point> Editor::selectedPoints() const {
  QSet<Point> all;
  for (Layer l: layers())
    all |= d->purepts[l] | d->selpts[l];
  return all;
}

Group const &Editor::currentGroup() const {
  return d->layout.root().subgroup(d->crumbs);
}

void Editor::rotateCW(bool noundo) {
  Group &here(d->currentGroup());
  Rect box(d->selectionBounds());
  if (box.isEmpty())
    return;
  Point center = box.center(); //.roundedTo(d->layout.board().grid);
  UndoCreator uc(d);
  if (!noundo && (!d->selection.isEmpty() || !selectedPoints().isEmpty()))
    uc.realize();
  for (int id: d->selection)
    here.object(id).rotateCW(center);

  Dim mrg = d->pressMargin();
  for (Point p: selectedPoints()) {
    // Rotate end points of traces if those end points are in purepts,
    // but the trace itself is not in selection.
    for (int id: here.keys()) {
      if (!d->selection.contains(id)) {
	Object &obj(here.object(id));
	if (obj.isTrace()) {
	  Trace &t(obj.asTrace());
	  if (t.onP1(p, mrg))
	    t.p1.rotateCW(center);
	  if (t.onP2(p, mrg))
	    t.p2.rotateCW(center);
	}
      }
    }
  }

  for (Layer l: layers()) {
    QSet<Point> newpts;
    for (Point const &p: d->purepts[l])
      newpts << p.rotatedCW(center);
    d->purepts[l] = newpts;
    newpts.clear();
    for (Point const &p: d->selpts[l])
      newpts << p.rotatedCW(center);
    d->selpts[l] = newpts;
  }
}

void Editor::rotateCCW(bool noundo) {
  rotateCW(noundo);
  rotateCW(true);
  rotateCW(true);
  update();
}

void Editor::translate(Point dp) {
  if (d->selection.isEmpty() && selectedPoints().isEmpty())
    return;

  UndoCreator uc(d, true);

  Group &here(d->currentGroup());
  
  for (int id: d->selection)
    here.object(id).translate(dp);

  Dim mrg = d->pressMargin();
  for (Point p: selectedPoints()) {
    // Translate end points of traces if those end points are in purepts,
    // but the trace itself is not in selection.
    for (int id: here.keys()) {
      if (!d->selection.contains(id)) {
	Object &obj(here.object(id));
	if (obj.isTrace()) {
	  Trace &t(obj.asTrace());
	  if (t.onP1(p, mrg))
	    t.p1 += dp;
	  if (t.onP2(p, mrg))
	    t.p2 += dp;
	}
      }
    }
  }
  
  // update purepts and selpts  
  for (Layer l: layers()) {
    QSet<Point> newpts;
    for (Point const &p: d->purepts[l])
      newpts << p + dp;
    d->purepts[l] = newpts;
    newpts.clear();
    for (Point const &p: d->selpts[l])
      newpts << p + dp;
    d->selpts[l] = newpts;
  }
}

void Editor::flipH(bool noundo) {
  Rect box(d->selectionBounds());
  if (box.isEmpty())
    return;
  Dim center = box.center().x; //.roundedTo(d->layout.board().grid).x;

  UndoCreator uc(d);
  if (!noundo && (!d->selection.isEmpty() || !selectedPoints().isEmpty()))
    uc.realize();

  Group &here(d->currentGroup());
  
  for (int id: d->selection)
    here.object(id).flipLeftRight(center);

  Dim mrg = d->pressMargin();
  for (Point p: selectedPoints()) {
    // Rotate end points of traces if those end points are in purepts,
    // but the trace itself is not in selection.
    for (int id: here.keys()) {
      if (!d->selection.contains(id)) {
	Object &obj(here.object(id));
	if (obj.isTrace()) {
	  Trace &t(obj.asTrace());
	  if (t.onP1(p, mrg))
	    t.p1.flipLeftRight(center);
	  if (t.onP2(p, mrg))
	    t.p2.flipLeftRight(center);
	}
      }
    }
  }
  for (Layer l: layers()) {
    QSet<Point> newpts;
    for (Point const &p: d->purepts[l])
      newpts << p.flippedLeftRight(center);
    d->purepts[l] = newpts;
    newpts.clear();
    for (Point const &p: d->selpts[l])
      newpts << p.flippedLeftRight(center);
    d->selpts[l] = newpts;
  }
}

void Editor::flipV() {
  rotateCW();
  flipH(true);
  rotateCCW(true);
  update();
}

EProps &Editor::properties() {
  return d->props;
}

void Editor::formGroup() {
  if (d->selection.isEmpty())
    return;
  Group &here(d->currentGroup());
  UndoCreator uc(d, true);
  here.formSubgroup(d->selection);
  clearSelection();
  emit componentsChanged();
}

void Editor::dissolveGroup() {
  if (d->selection.isEmpty())
    return;
  UndoCreator uc(d, true);
  Group &here(d->currentGroup());
  for (int id: d->selection) 
    if (here.object(id).isGroup())
      here.dissolveSubgroup(id);
  emit componentsChanged();
  clearSelection();
}

void Editor::deleteSelected() {
  if (d->planeeditor) {
    d->planeeditor->deleteSelected();
    return;
  }
  if (d->selection.isEmpty())
    return;
  UndoCreator uc(d, true);
  Group &here(d->currentGroup());
  bool compchg = false;
  for (int id: d->selection) {
    if (here.object(id).isGroup()) {
      compchg = true;
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
  if (compchg)
    emit componentsChanged();
  clearSelection();
}
  
int Editor::selectedComponent(QString *msg) const {
  Group &here(d->currentGroup());
  int gid = 0;
  int tid = 0;
  QString m = "Nothing selected";
  for (int id: d->selection) {
    Object const &obj(here.object(id));
    if (obj.isGroup()) {
      if (gid && gid!=id) {
        m = "More than one group selected";
        gid = 0;
        break;
      } else {
        gid = id;
        int tid1 = obj.asGroup().refTextId();
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
      if (gid1==0 || (gid && gid1!=gid)) {
        m = "More than one item selected";
        gid = 0;
        break;
      } else {
        tid = id;
        gid = gid1;
      }
    } else {
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

bool Editor::saveComponent(int id, QString fn) {
  bool ok = d->layout.root().subgroup(d->crumbs).saveComponent(id, fn);
  if (ok)
    d->layout.root().subgroup(d->crumbs).object(id).asGroup().pkg
      = QFileInfo(fn).baseName();
  return ok;
}

bool Editor::insertComponent(QString fn, Point pt) {
  pt = pt.roundedTo(d->layout.board().grid);
  Group &here(d->currentGroup());
  UndoCreator uc(d, true);
  int gid = here.insertComponent(fn);
  if (!gid)
    return false;
  Object &obj(here.object(gid));
  Point delta = pt - obj.asGroup().anchor();
  obj.translate(delta);
  int tid = obj.asGroup().refTextId();
  if (here.contains(tid))
    here.object(tid).translate(delta);
  select(gid, false);
  select(tid, true);
  emit componentsChanged();
  return true;
}

Point Editor::hoverPoint() const {
  return d->hoverpt;
}

void Editor::dragEnterEvent(QDragEnterEvent *e) {
  QMimeData const *md = e->mimeData();
  if (md->hasFormat(ComponentView::dndformat)) {
    e->accept();
  } else if (md->hasUrls()) {
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
  if (md->hasFormat(ComponentView::dndformat)) {
    int id = QString(md->data(ComponentView::dndformat)).toInt();
    ElementView const *src = ElementView::instance(id);
    Group grp(src->group());
    Point droppos = Point::fromMils(d->widget2mils.map(e->pos()));
    QString ref = src->refText();
    QString pv = src->pvText();
    grp.ref = ref;
    grp.setRefTextId(0);
    UndoCreator uc(d, true);
    Group &here(d->currentGroup());
    Point anch = grp.anchor(); // this should be at droppos
    Object obj(grp);
    obj.translate(droppos - anch);
    int gid = here.insert(obj);
    here.ensureRefText(gid);
    emit componentsChanged();
    update();
    e->accept();
  } else if (md->hasUrls()) {
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

bool Editor::linkSchematic(QString fn) {
  d->linkedschematic.link(fn);
  if (d->linkedschematic.isValid()) {
    d->layout.board().linkedschematic = fn;
    emit schematicLinked(true);
    return true;
  } else {
    d->layout.board().linkedschematic = "";
    emit schematicLinked(false);
    return false;
  }
}

void Editor::unlinkSchematic() {
  d->linkedschematic.unlink();
  d->layout.board().linkedschematic = "";
  emit schematicLinked(false);
}

LinkedSchematic const &Editor::linkedSchematic() const {
  return d->linkedschematic;
}

void Editor::undo() {
  if (d->undostack.isEmpty())
    return;
  { UndoStep s;
    s.layout = d->layout;
    s.selection = d->selection;
    s.selpts = d->selpts;
    s.purepts = d->purepts;
    d->redostack << s;
  }
  { UndoStep const &s = d->undostack.last();
    d->layout = s.layout;
    d->selection = s.selection;
    d->selpts = s.selpts;
    d->purepts = s.purepts;
  }
  d->invalidateStuckPoints();
  d->undostack.removeLast();
  d->stepsfromsaved--;
  emit changedFromSaved(d->stepsfromsaved != 0);
  d->emitSelectionStatus();
  emit componentsChanged();
  emit boardChanged(d->layout.board());
  emit undoAvailable(!d->undostack.isEmpty());
  emit redoAvailable(true);
  update();
}

void Editor::redo() {
  if (d->redostack.isEmpty())
    return;
  { UndoStep s;
    s.layout = d->layout;
    s.selection = d->selection;
    s.selpts = d->selpts;
    s.purepts = d->purepts;
    d->undostack << s;
  }
  { UndoStep const &s = d->redostack.last();
    d->layout = s.layout;
    d->selection = s.selection;
    d->selpts = s.selpts;
    d->purepts = s.purepts;
  }
  d->invalidateStuckPoints();
  d->redostack.removeLast();
  d->stepsfromsaved++;
  emit changedFromSaved(d->stepsfromsaved != 0);
  d->emitSelectionStatus();
  emit componentsChanged();
  emit boardChanged(d->layout.board());
  emit undoAvailable(true);
  emit redoAvailable(!d->redostack.isEmpty());
  update();
}

bool Editor::isUndoAvailable() const {
  return !d->undostack.isEmpty();
}

bool Editor::isRedoAvailable() const {
  return !d->redostack.isEmpty();
}

bool Editor::isAsSaved() const {
  return d->stepsfromsaved == 0;
}


void Editor::markAsSaved() {
  d->stepsfromsaved = 0;
}

void Editor::cut() {
  copy();
  deleteSelected();
}

void Editor::copy() {
  Clipboard &clp(Clipboard::instance());
  Group const &here(d->currentGroup());
  clp.store(here, d->selection);
}

void Editor::paste() {
  Clipboard &clp(Clipboard::instance());
  if (!clp.isValid())
    return;
  clearSelection();
  UndoCreator uc(d, true);
  Group &here(d->currentGroup());
  QSet<int> ids = here.merge(clp.retrieve());
  for (int id: ids)
    select(id, true);
  d->updateOnWhat(true);
  emit componentsChanged();
}

PlaneEditor *Editor::planeEditor() const {
  return d->planeeditor;
}

void Editor::deleteDanglingTraces() {
  Group here = currentGroup();
  TraceRepair tr(here);
  if (tr.dropDanglingTraces()) {
    clearSelection();
    UndoCreator uc(d, true);
    d->currentGroup() = here;
    d->updateOnWhat(true);
    update();
  }
}
