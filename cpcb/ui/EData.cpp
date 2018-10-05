// EData.cpp

#include "EData.h"
#include "Editor.h"
#include "Tracer.h"

#include "UndoCreator.h"
#include "PinNameEditor.h"
#include "svg/Symbol.h"

#include <QMessageBox>

constexpr int MOVETHRESHOLD_PIX = 4;
constexpr int MARGIN_PIX = 4;

EData::EData(Editor *ed): ed(ed) {
  autofit = false;
  mode = Mode::Edit;
  moving = false;
  panning = false;
  rubberband = 0;
  stuckptsvalid = false;
  stepsfromsaved = false;
  netsvisible = true;
  resizeTimer = 0;
  tracer = 0;
  planeeditor = 0;
}

Dim EData::pressMargin() const {
  return Dim::fromMils(MARGIN_PIX/mils2px);
}

Group const &EData::currentGroup() const {
  return layout.root().subgroup(crumbs);
}

Group &EData::currentGroup() {
  return layout.root().subgroup(crumbs);
}

bool EData::updateOnWhat(bool force) {
  Dim mrg = pressMargin();
  NodeID ids = visibleNodeAt(hoverpt, mrg);
  bool isnew = ids != onnode;
  onnode = ids;
  if (isnew || force)
    onobject = currentGroup().humanName(ids);
  if (netsvisible && (isnew || force))
    updateNet(ids);
  return isnew;
}

void EData::updateNet(NodeID seed) {
  net = PCBNet(layout.root().subgroup(crumbs), seed);

  linkednet = LinkedNet();
  if (linkedschematic.isValid() && !net.nodes().isEmpty() && crumbs.isEmpty()) {
    Nodename seed = net.someNode();
    // qDebug() << "updatenet" << seed;
    for (LinkedNet const &lnet: linkedschematic.nets()) {
      // qDebug() << "  comparing against" << lnet;
      if (lnet.containsMatch(seed)) {
	linkednet = lnet;
	break;
      }
    }
  }

  if (linkedschematic.isValid() && crumbs.isEmpty()) 
    netmismatch.recalculate(net, linkednet, layout.root());
  else
    netmismatch.reset();
}

void EData::invalidateStuckPoints() const {
  stuckptsvalid = false;
}

void EData::validateStuckPoints() const {
  if (stuckptsvalid)
    return;
  // stuckpts will be the | of all points of nonselected nontraces, but only
  // if those points are also in selpts. (Others are irrelevant.)
  stuckpts.clear();
  Group const &here(currentGroup());
  auto const &lays(::layers());
  for (int id: here.keys()) {
    if (!selection.contains(id)) {
      Object const &obj(here.object(id));
      if (!obj.isTrace())
	for (Layer l: lays)
	  stuckpts[l] |= obj.allPoints(l);
    }
  }
  for (Layer l: lays)
    stuckpts[l] &= selpts[l];
  stuckptsvalid = true;
}

Rect EData::selectionBounds() const {
  Group const &here(currentGroup());
  
  Rect r;
  for (int id: selection)
    r |= here.object(id).boundingRect();

  for (Layer l: ::layers()) 
    for (Point p: purepts[l])
      r |= p;

  return r;
}
  
void EData::createUndoPoint() {
  UndoStep s;
  s.layout = layout;
  s.selection = selection;
  s.selpts = selpts;
  s.purepts = purepts;
  undostack << s;
  redostack.clear();
  stepsfromsaved ++;
  ed->undoAvailable(true);
  ed->redoAvailable(false);
  ed->changedFromSaved(stepsfromsaved != 0);
}

void EData::selectPointsOf(int id) {
  Group const &here(currentGroup());
  if (here.contains(id))
    for (Layer l: ::layers())
      selpts[l] |= here.object(id).allPoints(l);
}

void EData::drawBoard(QPainter &p) const {
  p.setBrush(QBrush(QColor(0,0,0)));
  double lw = layout.board().width.toMils();
  double lh = layout.board().height.toMils();
  if (layout.board().shape == Board::Shape::Round) {
    if (lw==lh) {
      p.drawEllipse(QPointF(lw/2, lh/2), lw/2, lh/2);
    } else if (lw<lh) {
      p.setPen(QPen(QColor(0,0,0), lw, Qt::SolidLine, Qt::RoundCap));
      p.drawLine(QPointF(lw/2, lw/2), QPointF(lw/2, lh - lw/2));
    } else {
      p.setPen(QPen(QColor(0,0,0), lh, Qt::SolidLine, Qt::RoundCap));
      p.drawLine(QPointF(lh/2, lh/2), QPointF(lw - lh/2, lh/2));
    }
  } else {
    p.drawRect(QRectF(QPointF(0,0), QPointF(lw, lh)));
  }
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

void EData::drawObjects(QPainter &p) const {
  Group const &here(currentGroup());
  validateStuckPoints();  

  Board const &brd = layout.board();
  ORenderer rndr(&p);
  rndr.setBoard(brd);
  if (moving)
    rndr.setMoving(movingdelta);
  rndr.setSelPoints(selpts);
  rndr.setPurePoints(purepts);
  rndr.setStuckPoints(stuckpts);

  auto onesublayer = [&](Layer l, ORenderer::Sublayer s) {
    rndr.setLayer(l, s);
    for (int id: here.keys()) {
      QSet<NodeID> subnet;
      if (netsvisible)
	for (NodeID nid: net.nodes())
	  if (!nid.isEmpty() && nid.first()==id)
	    subnet << nid.tail();
      rndr.drawObject(here.object(id), selection.contains(id) && !planeeditor,
                      subnet);
    }
  };
  auto onelayer = [&](Layer l) {
    if (brd.planesvisible) {
      if (l==Layer::Top) {
        QPixmap pm(ed->size());
        pm.fill(QColor(0, 0, 0, 0));
        QPainter pmp(&pm);
        rndr.setPainter(&pmp);
        pmp.setTransform(mils2widget, true);
        pmp.setCompositionMode(QPainter::CompositionMode_Source);
        onesublayer(l, ORenderer::Sublayer::Plane);
        onesublayer(l, ORenderer::Sublayer::Clearance);
        onesublayer(l, ORenderer::Sublayer::Main);
        rndr.setPainter(&p);
        p.setCompositionMode(QPainter::CompositionMode_SourceOver);
        p.setTransform(QTransform());
        p.drawPixmap(QPoint(0,0), pm);
        p.setTransform(mils2widget, true);
      } else if (l==Layer::Bottom) {
        p.setCompositionMode(QPainter::CompositionMode_Source);
        onesublayer(l, ORenderer::Sublayer::Plane);
        onesublayer(l, ORenderer::Sublayer::Clearance);
        onesublayer(l, ORenderer::Sublayer::Main);
      } else {
        onesublayer(l, ORenderer::Sublayer::Main);
      }
    } else {
      onesublayer(l, ORenderer::Sublayer::Main);
    }
  };

  if (brd.layervisible[Layer::Bottom])
    onelayer(Layer::Bottom);

  if (brd.layervisible[Layer::Top])
    onelayer(Layer::Top);

  if (brd.layervisible[Layer::Silk])
    onelayer(Layer::Silk);

  if (netsvisible && crumbs.isEmpty())
    drawNetMismatch(rndr);

  onelayer(Layer::Invalid); // magic to punch holes
}

void EData::drawNetMismatch(ORenderer &rndr) const {
  rndr.setLayer(Layer::Silk, ORenderer::Sublayer::Main);
  rndr.setOverride(ORenderer::Override::WronglyIn);
  for (NodeID const &nid: netmismatch.wronglyInNet) {
    Object const &obj(layout.root().object(nid));
    rndr.drawObject(obj);
  }
  rndr.setOverride(ORenderer::Override::Missing);
  for (NodeID const &nid: netmismatch.missingFromNet) {
    Object const &obj(layout.root().object(nid));
    rndr.drawObject(obj);
  }
  rndr.setOverride(ORenderer::Override::None);
}

void EData::drawSelectedPoints(QPainter &p) const {
  QSet<Point> pts;
  for (Layer l: layers())
    pts |= purepts[l];
  if (pts.isEmpty())
    return;
  p.setPen(QPen(Qt::NoPen));
  p.setBrush(QColor(255, 255, 255, 128));
  for (Point pt: pts) {
    if (moving)
      pt += movingdelta;
    p.drawEllipse(pt.toMils(), 25, 25);
  }
}

void EData::abortTracing() {
  if (tracer) {
    tracer->end();
    delete tracer;
    tracer = 0;
    ed->updateOnNet();
    ed->update();
  }
}

void EData::pressText(Point p) {
  if (props.text.isEmpty())
    props.text = QInputDialog::getText(ed, "Place text", "Text:");
  if (props.text.isEmpty())
    return;

  p = p.roundedTo(layout.board().grid);
  Group &here(currentGroup());
  Text t;
  t.p = p;
  t.fontsize = props.fs;
  t.rota = props.rota;
  t.flip = props.flip;
  t.text = props.text;
  t.layer = props.layer;
  UndoCreator uc(this, true);
  here.insert(Object(t));
} 

void EData::pressNPHole(Point p) {
  p = p.roundedTo(layout.board().grid);
  Group &here(currentGroup());
  NPHole t;
  t.p = p;
  t.d = props.od;
  UndoCreator uc(this, true);
  here.insert(Object(t));
  ed->update();
}

void EData::pressHole(Point p) {
  p = p.roundedTo(layout.board().grid);
  if (props.od - props.id < Dim::fromInch(0.005)) {
    QMessageBox::warning(ed, "cpcb",
			 "Invalid ID/OD pairing",
			 QMessageBox::Ok);
  } else {
    Group &here(currentGroup());
    Hole t;
    t.p = p;
    t.od = props.od;
    t.id = props.id;
    t.square = props.square;
    t.ref = props.text;
    UndoCreator uc(this, true);
    here.insert(Object(t));
    ed->insertedPadOrHole();
  }
}


void EData::pressPad(Point p) {
  p = p.roundedTo(layout.board().grid);
  Group &here(currentGroup());
  Pad t;
  t.p = p;
  t.width = props.w;
  t.height = props.h;
  t.layer = props.layer;
  t.ref = props.text;
  UndoCreator uc(this, true);
  here.insert(Object(t));
  ed->insertedPadOrHole();
}

void EData::pressArc(Point p) {
  p = p.roundedTo(layout.board().grid);
  Group &here(currentGroup());
  Arc t;
  t.center = p;
  t.radius = props.id / 2;
  t.linewidth = props.linewidth;
  if (props.arcangle<0) {
    t.angle = -props.arcangle;
    t.rota = props.rota;
  } else {
    t.angle = props.arcangle;
    t.rota = props.rota;
    t.rota -= t.angle/2;
  }
  t.layer = props.layer;
  UndoCreator uc(this, true);
  here.insert(Object(t));
}

void EData::pressPickingUp(Point p) {
  if (tracer) {
    pressTracing(p);
    return;
  }

  tracer = new Tracer(this);
  tracer->pickup(p);
  if (tracer->isTracing()) 
    ed->update();
  else
    abortTracing();
}

void EData::pressTracing(Point p) {
  if (!tracer)
    tracer = new Tracer(this);
  tracer->click(p);
  if (!tracer->isTracing())
    abortTracing();
}

bool EData::isMoveSignificant(Point p) {
  if (significantmove)
    return true;

  significantmove = (mils2widget.map(p.toMils())
		     - mils2widget.map(presspoint.toMils()))
    .manhattanLength() > MOVETHRESHOLD_PIX;
  return significantmove;
}

void EData::moveMoving(Point p) {
  if (isMoveSignificant(p)) {
    movingdelta = p.roundedTo(layout.board().grid) - movingstart;
    ed->update();
  }
}

void EData::moveTracing(Point p) {
  if (!tracer)
    return;
  tracer->move(p);
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

NodeID EData::visibleNodeAt(Point p, Dim mrg) const {
  Group const &here(currentGroup());
  return visibleNodeAt(here, p, mrg);
}

NodeID EData::visibleNodeAt(Group const &grp, Point p, Dim mrg) const {
  NodeID nid;
  int id = visibleObjectAt(grp, p, mrg);
  if (id<0)
    return nid;

  nid << id;
  if (grp.object(id).isGroup()) 
    nid += visibleNodeAt(grp.object(id).asGroup(), p, mrg);
  return nid;
}

void EData::pressOrigin(Point p) {
  NodeID node = visibleNodeAt(p);
  Object const &obj(currentGroup().object(node));
  if (obj.isPad()) {
    userorigin = obj.asPad().p;
    ed->userOriginChanged(userorigin);
  } else if (obj.isHole()) {
    userorigin = obj.asHole().p;
    ed->userOriginChanged(userorigin);
  }
}

int EData::visibleObjectAt(Point p, Dim mrg) const {
  Group const &here(currentGroup());
  return visibleObjectAt(here, p, mrg);
}

int EData::visibleObjectAt(Group const &here, Point p, Dim mrg) const {
  QList<int> ids = here.objectsAt(p, mrg);
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
    Object const &obj = here.object(id);
    Layer l = obj.layer();
    switch (obj.type()) {
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
    case Object::Type::NPHole:
      p1 = Prio::Silk;
      break;
    case Object::Type::Group:
      p1 = Prio::Silk;
      break;
    case Object::Type::Plane:
      if (brd.planesvisible && brd.layervisible[l])
        p1 = l==Layer::Bottom ? Prio::BottomPlane
          : l==Layer::Top ? Prio::TopPlane
          : Prio::None;
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
  Dim mrg = pressMargin();
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
	startMoveSelection(fave);
      }
    } else {
      newSelectionUnless(fave, p, mrg, add);
      startMoveSelection(fave);
    }
  }
}

void EData::dropFromSelection(int id, Point p, Dim mrg) {
  // throw ID out of selection, then recollect SELPTS.
  selection.remove(id);
  selpts.clear();

  Object const &obj(layout.root().subgroup(crumbs).object(id));
  if (obj.isTrace()) {
    Trace const &t(obj.asTrace());
    if (t.onP1(p, mrg)) 
      purepts[t.layer].remove(t.p1);
    if (t.onP2(p, mrg)) 
      purepts[t.layer].remove(t.p2);
  }
  
  for (int k: selection)
    selectPointsOf(k);
  ed->update();
  emitSelectionStatus();
}

void EData::startMoveSelection(int fave) {
  moving = true;
  movingstart = presspoint.roundedTo(layout.board().grid);
  if (fave>0) {
    // if clicked on a pin (directly or in a component), align that pin
    // with the grid.
    Object const &obj = currentGroup().object(fave);
    if (obj.isHole()) {
      movingstart = obj.asHole().p;
    } else if (obj.isPad()) {
      movingstart = obj.asPad().p;
    } else if (obj.isGroup()) {
      Group const &grp(obj.asGroup());
      Dim mrg = pressMargin();
      fave = visibleObjectAt(grp, presspoint, mrg);
      if (fave>0) {
	Object const &obj = grp.object(fave);
	if (obj.isHole()) {
	  movingstart = obj.asHole().p;
	} else if (obj.isPad()) {
	  movingstart = obj.asPad().p;
	}
      }
    }
  }
  movingdelta = Point();
}

void EData::newSelectionUnless(int id, Point p, Dim mrg, bool add) {
  // does not clear purepts if on a purept
  Group const &here(currentGroup());
  Object const &obj(here.object(id));
  if (obj.isTrace()) {
    Trace const &t(obj.asTrace());
    if (t.onP1(p, mrg)) {
      if (purepts[t.layer].contains(t.p1)) {
	if (add)
	  ed->deselectPoint(t.p1);
      } else {
	ed->selectPoint(t.p1, add);
      }
    } else if (t.onP2(p, mrg)) {
      if (purepts[t.layer].contains(t.p2)) {
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
    if (obj.isGroup())
      ed->select(obj.asGroup().refTextId(), true);
  }
}


void EData::moveBanding(Point p) {
  if (!rubberband)
    return;
  if (mode==Mode::PlacePlane)
    p = p.roundedTo(layout.board().grid);
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
  movingdelta = p.roundedTo(layout.board().grid) - movingstart;
  if (movingdelta.isNull() || !isMoveSignificant(p)) {
    moving = false;
    ed->update();
    return;
  }
  validateStuckPoints();
  UndoCreator uc(this, true);
  Group &here(currentGroup());
  for (int id: here.keys()) {
    Object &obj(here.object(id));
    if (selection.contains(id)) {
      if (obj.isTrace()) {
        Trace &t = obj.asTrace();
        if (!stuckpts[t.layer].contains(t.p1))
          t.p1 += movingdelta;
        if (!stuckpts[t.layer].contains(t.p2))
          t.p2 += movingdelta;
      } else {
        obj.translate(movingdelta);
      }
    } else if (obj.isTrace()) {
      uc.realize();
      Trace &t = obj.asTrace();
      if ((selpts[t.layer].contains(t.p1)
	   || purepts[t.layer].contains(t.p1))
          && !stuckpts[t.layer].contains(t.p1))
	t.p1 += movingdelta;
      if ((selpts[t.layer].contains(t.p2)
	   || purepts[t.layer].contains(t.p2))
          && !stuckpts[t.layer].contains(t.p2))
	t.p2 += movingdelta;
    }
  }

  for (Layer l: ::layers()) {
    QSet<Point> newsel;
    for (Point const &pt: selpts[l])
      newsel << pt + movingdelta;
    selpts[l] = newsel;
    QSet<Point> newpure;
    for (Point const &pt: purepts[l])
      newpure << pt + movingdelta;
    purepts[l] = newpure;
  }
  
  moving = false;
  ed->update();
}

void EData::releaseBanding(Point p) {
  delete rubberband;
  rubberband = 0;
  switch (mode) {
  case Mode::Edit:
    ed->selectArea(Rect(presspoint, p), true);
    break;
  case Mode::PlacePlane: {
    p = p.roundedTo(layout.board().grid);
    if (p==presspoint)
      return;
    UndoCreator uc(this, true);
    FilledPlane fp;
    fp.layer = props.layer;
    fp.perimeter << presspoint;
    fp.perimeter << Point(presspoint.x, p.y);
    fp.perimeter << p;
    fp.perimeter << Point(p.x, presspoint.y);
    currentGroup().insert(Object(fp));
    ed->update();
  } break;
  default:
    qDebug() << "Surprise release banding";
    break;
  }
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
  ed->scaleChanged();
}

void EData::perhapsRefit() {
  QSize s = ed->size();
  if (abs(s.width() - lastsize.width()) < 3
      && abs(s.height() - lastsize.height()) < 3)
    return;
  if (autofit)
    ed->scaleToFit();
  lastsize = s;
}

void EData::emitSelectionStatus() {
  if (selection.isEmpty()) {
    ed->selectionChanged(false);
    ed->selectionIsGroup(false);
  } else {
    ed->selectionChanged(true);
    int gid = -1;
    int tid = -1;
    Group const &here(currentGroup());
    for (int id: selection) {
      Object const &obj(here.object(id));
      if (obj.isGroup()) {
	if (gid>0 || (tid>0 && obj.asGroup().refTextId()!=tid)) {
	  // already had group, or mismatching text
	  ed->selectionIsGroup(false);
	  return;
	} else {
	  gid = id;
	  tid = obj.asGroup().refTextId();
	}
      } else if (obj.isText()) {
	int g1 = obj.asText().groupAffiliation();
	if (g1>0 && (gid<0 || gid==g1)) { // matching text
	  tid = id;
	} else { // mismatching text
	  ed->selectionIsGroup(false);
	  return;
	}
      } else { // bad object type
	ed->selectionIsGroup(false);
	return;
      }	
    }
    ed->selectionIsGroup(gid>0);
  }
}

void EData::editPinName(int groupid, int hole_pad_id) {
  Group const &grp(currentGroup().object(groupid).asGroup());
  Object const &obj(grp.object(hole_pad_id));
  QString group_ref = grp.ref;
  bool ishole = obj.isHole();
  bool ispad = obj.isPad();
  Q_ASSERT(ishole || ispad);
  QString pin_ref = ishole ? obj.asHole().ref : obj.asPad().ref;
  bool ok = false;
  Symbol const &sym(linkedschematic.schematic()
		    .symbolForNamedElement(group_ref));
  if (sym.isValid()) {
    PinNameEditor pne(group_ref, pin_ref, sym);
    ok = pne.exec();
    if (ok)
      pin_ref = pne.pinRef();
  } else {
    QString what = ishole ? "Hole" : "Pad";
    pin_ref = QInputDialog::getText(ed, what + " properties", "Pin name/number:",
				    QLineEdit::Normal,
				    pin_ref, &ok);
  }
  if (ok) {
    UndoCreator uc(this, true);
    Group &group(currentGroup().object(groupid).asGroup());
    group.setPinRef(hole_pad_id, pin_ref);
  }
}
