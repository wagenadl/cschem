// Editor.cpp

#include "Editor.h"
#include "EData.h"
#include "UndoCreator.h"
#include "Tracer.h"
#include "PlaneEditor.h"
#include "data/TraceRepair.h"
#include "ORenderer.h"

#include <QInputDialog>
#include <QResizeEvent>
#include <QTimer>
#include <algorithm>

#include "ui/BOM.h"

Editor::Editor(QWidget *parent): QWidget(parent), d(new EData(this)) {
  QPalette p(palette());
  //  p.setColor(QPalette::Window, QColor(80, 80, 80));
  p.setColor(QPalette::Window, ORenderer::backgroundColor());
  setPalette(p);
  setAutoFillBackground(true);
  setMouseTracking(true);
  setAcceptDrops(true);
  setFocusPolicy(Qt::StrongFocus);
  scaleToFit();
  d->bom = new BOM(this);
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
  if (m!=Mode::Edit)
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
  if (m==Mode::Edit) 
    setCursor(Qt::ArrowCursor);
  else
    setCursor(d->crossCursor());
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
  emit schematicLinked(d->linkedschematic.isValid());
  emit boardChanged(d->layout.board());
  d->bom->rebuild();
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
  if (e->modifiers() & Qt::ControlModifier) {
    d->zoom(pow(2, e->angleDelta().y()/240.));
  } else {
    QPoint delta = e->pixelDelta()*2;
    d->mils2widget.translate(delta.x()/d->mils2px, delta.y()/d->mils2px);
    d->widget2mils = d->mils2widget.inverted();
    update();
  }
}

void Editor::resizeEvent(QResizeEvent *) {
  //  qDebug() << "editor::resizeevent" << e->oldSize() << e->size();
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
	d->pressTracing(p, e->modifiers());
	break;
      case Mode::PlaceHole:
	d->pressHole(p);
	break;
      case Mode::PlaceNPHole:
	d->pressNPHole(p);
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
	d->pressPickingUp(p, e->modifiers());
	break;
      case Mode::PlacePlane:
        Q_ASSERT(d->planeeditor);
        d->planeeditor->mousePress(p, e->button(), e->modifiers());
        break;
      case Mode::SetIncOrigin:
	d->pressOrigin(p);
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

void Editor::pretendOnNet(NodeID ids) {
  Nodename nn(d->currentGroup().nodeName(ids));
  Nodename alias(d->linkedschematic.pinAlias(nn));
  qDebug () << "pretendonnet" << nn << alias;
  if (alias.isValid())
    d->onobject += alias.humanName();
  else
    d->onobject = nn.humanName();
  if (d->netsvisible)
    d->updateNet(ids);
  emit onObject(d->onobject);
  QStringList mis;
  for (Nodename const &n: d->netmismatch.missingEntirely)
    mis << n.humanName();
  mis.sort();
  emit missingNodes(mis);
  if (d->netsvisible)
    update();
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
    } else if (d->mode==Mode::Edit) {
      if (e->modifiers() & Qt::ControlModifier)
	leaveAllGroups();
      else
	leaveGroup();
    } else {
      emit escapePressed();
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
      //if (fave>=0)
      //select(fave);
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
  if (sub<0) {
    // use selection, if a group
    Group &here(d->currentGroup());
    for (int id: d->selection) {
      if (here.object(id).isGroup()) {
	if (sub<0)
	  sub = id;
	else
	  return false;
      }
    }
  }
  clearSelection();
  Group const &here(d->currentGroup());
  if (here.contains(sub) && here.object(sub).isGroup()) {
    d->crumbs << sub;
    d->updateOnWhat(true);
    selectionChanged(false);  
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
  int gid = d->crumbs.takeLast();
  d->currentGroup().ensureRefText(gid);
  selectionChanged(false);
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

void Editor::select(QSet<int> ids) {
  d->invalidateStuckPoints();
  Group &here(d->currentGroup());

  d->selection.clear();
  d->selpts.clear();
  d->purepts.clear();

  for (int id: ids) {
    if (here.contains(id)) {
      d->selection.insert(id);
      d->selectPointsOf(id);
    }
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
      Arc &arc(obj.asArc());
      int a0 = (arc.rota + arc.angle/2) / 90.0 + .49;
      qDebug() << "setarcangle" << arc.rota << arc.angle << a0 << angle;
      if (angle<0) {
        arc.angle = -angle;
        arc.rota = FreeRotation(a0*90);
      } else {
        arc.angle = angle;
        arc.rota = FreeRotation(a0*90 - angle/2);
      }
      qDebug() << " to " << arc.rota << arc.angle << a0*90 - angle/2
               << FreeRotation(210);
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

  Group const &here(d->currentGroup());
  UndoCreator uc(d);
  for (int id: d->selection) {
    Object const &obj(here.object(id));
    switch (obj.type()) {
    case Object::Type::Trace:
      uc.realize();
      d->currentGroup().adjustViasAroundTrace(id, l);
      d->currentGroup().object(id).asTrace().layer = l;
      break;
    case Object::Type::Text:
      if (obj.asText().groupAffiliation()<0) {
	uc.realize();
	d->currentGroup().object(id).asText().setLayer(l);
      }
      break;
    case Object::Type::Pad:
      uc.realize();
      d->currentGroup().object(id).asPad().layer = l;
      break;
    case Object::Type::Arc:
      uc.realize();
      d->currentGroup().object(id).asArc().layer = l;
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
    } else if (obj.isNPHole()) {
      uc.realize();
      obj.asNPHole().d = x;
    } else if (obj.isArc()) {
      uc.realize();
      obj.asArc().radius = x/2;
    }
  }
}

void Editor::setSlotLength(Dim x) {
  d->props.slotlength = x;
  UndoCreator uc(d);

  Group &here(d->currentGroup());
  for (int id: d->selection) {
    Object &obj(here.object(id));
    if (obj.type()==Object::Type::Hole) {
      uc.realize();
      obj.asHole().slotlength = x;
    } else if (obj.type()==Object::Type::NPHole) {
      uc.realize();
      obj.asNPHole().slotlength = x;
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
  d->props.rota = FreeRotation(rot);
}

void Editor::setFlipped(bool f) {
  d->props.flip = f;
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

void Editor::arbitraryRotation(int degCW) {
  Group &here(d->currentGroup());
  Rect box(d->selectionBounds());
  if (box.isEmpty())
    return;
  Point center = box.center(); //.roundedTo(d->layout.board().grid);
  UndoCreator uc(d);
  if (!d->selection.isEmpty() || !selectedPoints().isEmpty())
    uc.realize();
  for (int id: d->selection)
    here.object(id).freeRotate(degCW, center);
}  

void Editor::rotateCW(bool noundo, bool nottext) {
  Group &here(d->currentGroup());
  Rect box(d->selectionBounds());
  if (box.isEmpty())
    return;
  Point center = box.center(); //.roundedTo(d->layout.board().grid);
  UndoCreator uc(d);
  if (!noundo && (!d->selection.isEmpty() || !selectedPoints().isEmpty()))
    uc.realize();
  if (d->selection.size()==1)
    nottext=false;
  for (int id: d->selection)
    here.object(id).rotateCW(center, nottext);

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

void Editor::rotateCCW(bool noundo, bool nottext) {
  rotateCW(noundo, nottext);
  rotateCW(true, nottext);
  rotateCW(true, nottext);
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

void Editor::flipH(bool noundo, bool nottext) {
  Rect box(d->selectionBounds());
  if (box.isEmpty())
    return;
  Dim center = box.center().x; //.roundedTo(d->layout.board().grid).x;

  UndoCreator uc(d);
  if (!noundo && (!d->selection.isEmpty() || !selectedPoints().isEmpty()))
    uc.realize();

  Group &here(d->currentGroup());
  if (d->selection.size()==1)
    nottext=false;  
  for (int id: d->selection)
    here.object(id).flipLeftRight(center, nottext);

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

void Editor::flipV(bool noundo, bool nottext) {
  rotateCW(noundo, nottext);
  flipH(true, nottext);
  rotateCCW(true, nottext);
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
      int gid = here.object(id).asText().groupAffiliation();
      here.object(gid).asGroup().setRefTextId(0);
      here.remove(id);
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

Group const &Editor::selectedComponentGroup() const {
  static Group g0;
  int id = selectedComponent();
  Object const &obj(currentGroup().object(id));
  if (obj.isGroup())
    return obj.asGroup();
  else
    return g0;
}

bool Editor::saveComponent(int id, QString fn) {
  Object const &obj(currentGroup().object(id));
  if (!obj.isGroup())
    return false;
  Group const &grp(obj.asGroup());
  int oldrot = grp.nominalRotation();
  QString oldpkg = grp.pkg;
  UndoCreator uc(d);
  if (oldrot || oldpkg=="")
    uc.realize(); // the rotation and/or pkg name is about to change
  
  bool ok = d->layout.root().subgroup(d->crumbs).saveComponent(id, fn);
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
      if (url.isLocalFile()) {
        QString fn1 = url.toLocalFile();
        if (fn1.toLower().endsWith(".svg")) {
          fn = fn1;
          break;
        }
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
    int tid = here.ensureRefText(gid);
    select(gid);
    if (tid>0)
      select(tid, true);
    emit componentsChanged();
    update();
    e->accept();
  } else if (md->hasUrls()) {
    QList<QUrl> urls = md->urls();
    bool take = false;
    clearSelection();
    for (QUrl url: urls) {
      if (url.isLocalFile()) {
        QString fn1 = url.toLocalFile();
        if (fn1.toLower().endsWith(".svg"))
          take = insertComponent(fn1,
			       Point::fromMils(d->widget2mils.map(e->pos())));
      }
    }
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

QString Editor::linkedSchematicFilename() const {
  if (d->linkedschematic.isValid())
    return d->layout.board().linkedschematic;
  else
    return QString();
}

bool Editor::linkSchematic(QString fn) {
  d->linkedschematic.link(fn);
  if (d->linkedschematic.isValid()) {
    d->layout.board().linkedschematic = fn;
    d->bom->rebuild();
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


bool Editor::isAsSaved() const {
  return d->stepsfromsaved == 0;
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
  Group pst(clp.retrieve());
  if (pst.isEmpty())
    return;
  
  clearSelection();
  UndoCreator uc(d, true);

  // Translate items to be pasted to mouse position
  QSet<Point> pp0 = pst.allPoints();
  if (pp0.isEmpty())
    pp0 = pst.altCoords();
  Point p1 = d->hoverpt.roundedTo(d->layout.board().grid);
  Point p0;
  if (pp0.size()) {
    /* Which point of the selection should be placed on p1? Top-left? Median?
       Let's use median.
    */
    QVector<Dim> xx, yy;
    for (Point const &p: pp0) {
      xx << p.x;
      yy << p.y;
    }
    int n = (xx.size()-1)/2;
    std::nth_element(xx.begin(), xx.begin()+n, xx.end());
    std::nth_element(yy.begin(), yy.begin()+n, yy.end());
    p0 = Point(xx[n], yy[n]);
  }
  pst.translate(p1 - p0);

  Group &here(d->currentGroup());
  QSet<int> ids = here.merge(pst);

  for (int id: ids)
    select(id, true);
  d->updateOnWhat(true);
  emit componentsChanged();
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

void Editor::cleanupIntersections() {
  Group here = currentGroup();
  TraceRepair tr(here);
  bool tri = tr.fixAllTraceIntersections(pcbLayout().board().grid);
  bool pni = tr.fixAllPinTouchings();
  if (tri || pni) {
    clearSelection();
    UndoCreator uc(d, true);
    d->currentGroup() = here;
    d->updateOnWhat(true);
    update();
  }
}

void Editor::setBoardSize(Dim w, Dim h, Board::Shape shp) {
  d->layout.board().width = w;
  d->layout.board().height = h;
  d->layout.board().shape = shp;
  qDebug() << "setboardsize should create an undo step";
  update();
  emit boardChanged(d->layout.board());
}

Point Editor::userOrigin() const {
  return d->userorigin;
}

void Editor::selectTrace(bool wholenet) {
  QSet<int> newids;
  Group const &here = d->currentGroup();

  auto ids_at = [](Group const &here, Point p, int id, Layer l, bool wholenet) {
    // this does not connect through pins in groups
    QSet<int> atp;
    bool hashole = false;
    if (wholenet)
      for (int id1: here.objectsAt(p))
	if (here.object(id1).isHole())
	  hashole = true;
    for (int id1: here.objectsAt(p)) {
      if (here.object(id1).isTrace()) {
	if (id1!=id && (hashole || here.object(id1).asTrace().layer==l))
	  atp << id1;
      } else if (!wholenet && !here.object(id1).isPlane()) {
	return QSet<int>();
      }
    }
    if (wholenet || atp.size()==1)
      return atp;
    else
      return QSet<int>();
  };

  QSet<int> ids = d->selection;
  QSet<int> allids = ids;
  while (!ids.isEmpty()) {
    for (int id: ids) {
      Object const &obj(here.object(id));
      if (obj.isTrace()) {
	Trace const &tr(obj.asTrace());
	newids |= ids_at(here, tr.p1, id, tr.layer, wholenet);
	newids |= ids_at(here, tr.p2, id, tr.layer, wholenet);
      }
    }
    newids -= allids;
    for (int id: newids) 
      select(id, true);
    allids |= newids;
    ids = newids;
    newids.clear();
  }
}

void Editor::setGroupPackage(QString t) {
  NodeID nodeid = breadcrumbs();
  if (nodeid.size()==1)
    d->bom->setData(d->bom->index(d->bom->findElement(nodeid[0]),
                                  int(BOM::Column::Package)), t);
  setGroupPackage(breadcrumbs(), t);
}

void Editor::setGroupPackage(NodeID path, QString t) {
  if (t != d->layout.root().subgroup(path).pkg) {
    UndoCreator uc(d, true);
    d->layout.root().subgroup(path).pkg = t;
  }
}

void Editor::setGroupPartno(QString t) {
  NodeID nodeid = breadcrumbs();
  if (nodeid.size()==1)
    d->bom->setData(d->bom->index(d->bom->findElement(nodeid[0]),
                                  int(BOM::Column::PartNo)), t);
  setGroupPartno(breadcrumbs(), t);
}

void Editor::setGroupPartno(NodeID path, QString t) {
  if (t != d->layout.root().subgroup(path).partno) {
    UndoCreator uc(d, true);
    d->layout.root().subgroup(path).partno = t;
  }
}

void Editor::setGroupNotes(QString t) {
  NodeID nodeid = breadcrumbs();
  if (nodeid.size()==1)
    d->bom->setData(d->bom->index(d->bom->findElement(nodeid[0]),
                                  int(BOM::Column::Notes)), t);
  setGroupNotes(breadcrumbs(), t);
}

void Editor::setGroupNotes(NodeID path, QString t) {
  if (t != d->layout.root().subgroup(path).notes) {
    UndoCreator uc(d, true);
    d->layout.root().subgroup(path).notes = t;
  }
}

BOM *Editor::bom() const {
  return d->bom;
}

bool Editor::loadBOM(QString fn) {
  leaveAllGroups();
  
  QList<BOMRow> rows(bom()->readAndVerifyCSV(fn));
  if (rows.isEmpty())
    return false;

  UndoCreator uc(d, true);

  QMap<QString, BOMRow> byref;
  for (BOMRow const &row: rows)
    byref[row.ref] = row;
  Group &root(d->layout.root());
  for (int k: root.keys()) {
    Object &obj(root.object(k));
    if (obj.isGroup()) {
      Group &grp(obj.asGroup());
      if (byref.contains(grp.ref)) {
        BOMRow row = byref[grp.ref];
        grp.notes = row.notes;
        grp.pkg = row.pkg;
        grp.partno = row.partno;
      }
    }
  }
  bom()->rebuild();
  return true;    
}
