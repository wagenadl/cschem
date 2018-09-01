// PlaneEditor.cpp

#include "PlaneEditor.h"
#include "Editor.h"
#include "UndoCreator.h"
#include "EData.h"
#include "data/Object.h"
#include "data/Point.h"
#include "data/FilledPlane.h"

class PEData {
public:
  PEData(EData *ed): ed(ed) {
    movingptidx = -1;
    movingedgeidx = -1;
    realmove = false;
    startmove = false;
  }
public:
  EData *ed;
  Point hoverpt;
  NodeID hovernode;
  Dim mrg;
  Point presspt;
  int movingptidx;
  int movingedgeidx;
  bool realmove;
  bool startmove;
public:
  int onVertex() const;
  int onEdge() const;
};

PlaneEditor::PlaneEditor(EData *ed): ed(ed), d(new PEData(ed)) {
  resetMouseMargin();
}

PlaneEditor::~PlaneEditor() {
  delete d;
}

void PlaneEditor::resetMouseMargin() {
  d->mrg = ed->layout.board().grid + ed->pressMargin();
}

void PlaneEditor::deleteSelected() {
  qDebug() << "PE:Delete";
  Group const &here(ed->currentGroup());
  Object const &obj(here.object(d->hovernode));
  if (obj.isPlane()) {
    UndoCreator(ed, true);
    Group &grp(ed->currentGroup().parentOf(d->hovernode));
    grp.remove(d->hovernode.last());
    d->hovernode = NodeID();
    ed->ed->update();
  }
}

int PEData::onVertex() const {
  Group const &here = ed->currentGroup();
  Object const &obj = here.object(hovernode);
  if (obj.isPlane()) {
    FilledPlane const &fp(obj.asPlane());
    int N = fp.perimeter.size();
    int nbest = -1;
    Dim dst = Dim::infinity();
    for (int n=0; n<N; n++) {
      Dim dst1 = hoverpt.distance(fp.perimeter[n]);
      if (dst1 < dst) {
        dst = dst1;
        nbest = n;
      }
    }
    if (dst < mrg) 
      return nbest;
  }
  return -1;
}
  
int PEData::onEdge() const {
  return -1; // soon...
}

void PlaneEditor::mousePress(Point p, Qt::MouseButton b,
                             Qt::KeyboardModifiers m) {
  mouseMove(p, b, m);
  if (d->hovernode.isEmpty()) {
    // not on a node => create a new plane
    p = p.roundedTo(ed->layout.board().grid);
    ed->presspoint = p;
    if (!ed->rubberband)
      ed->rubberband = new QRubberBand(QRubberBand::Rectangle, ed->ed);
    ed->rubberband->show();
    ed->rubberband->setGeometry(QRectF(ed->mils2widget.map(p.toMils()),
                                       QSize(0,0))
                                .toRect());
  } else {
    d->movingptidx = d->onVertex();
    d->movingedgeidx = d->movingptidx<0 ? d->onEdge() : -1;
    d->realmove = false;
    d->startmove = d->movingptidx>=0 || d->movingedgeidx>=0;
    d->presspt = p;
    qDebug() << "Press on " << d->movingptidx << d->movingedgeidx;
  }
}

void PlaneEditor::mouseRelease(Point p,
                               Qt::MouseButton b,
                               Qt::KeyboardModifiers m) {
  mouseMove(p, b, m);
  d->realmove = false;
  d->startmove = false;
}

void PlaneEditor::mouseMove(Point p,
                            Qt::MouseButton,
                            Qt::KeyboardModifiers) {
  d->hoverpt = p;
  if (ed->rubberband)
    return;
  if (d->startmove) {
    if (p.distance(d->presspt) > ed->pressMargin()) {
      d->realmove = true;
      d->startmove = false;
      UndoCreator uc(ed, true);
    }
  }
  if (d->realmove) {
    Group &here = ed->currentGroup();
    Object &obj = here.object(d->hovernode);
    if (!obj.isPlane()) {
      qDebug() << "PE move not on object??";
      return;
    }
    FilledPlane &fp(obj.asPlane());
    if (d->movingptidx>=0) {
      fp.perimeter[d->movingptidx] = p.roundedTo(ed->layout.board().grid);
      ed->ed->update();
    }
  }
  if (d->startmove || d->realmove)
    return;
  if (ed->onnode != d->hovernode) {
    d->hovernode = ed->onnode;
    if (ed->onnode.size()==1) {
      Object const &obj(ed->currentGroup().object(d->hovernode));
      if (obj.isPlane()) 
        ed->ed->select(ed->onnode[0]);
      else
        ed->ed->clearSelection();
    } else {
      ed->ed->clearSelection();
    }
  }
}

void PlaneEditor::doubleClick(Point p,
                              Qt::MouseButton,
                              Qt::KeyboardModifiers) {
  Group const &here(ed->currentGroup());
  NodeID nid = here.nodeAt(p, ed->pressMargin(), ed->props.layer, true);
  if (nid.isEmpty())
    return;
  Object const &obj(here.object(nid));
  if (obj.isPad()) {
    UndoCreator uc(ed, true);
    Pad &pad(ed->currentGroup().object(nid).asPad());
    pad.fpcon = !pad.fpcon;
    ed->updateOnWhat(true);
    ed->ed->update();
  } else if (obj.isHole()) {
    UndoCreator uc(ed, true);
    Hole &hole(ed->currentGroup().object(nid).asHole());
    if (hole.fpcon==ed->props.layer)
      hole.fpcon = Layer::Invalid;
    else
      hole.fpcon = ed->props.layer;
    ed->updateOnWhat(true);
    ed->ed->update();
  }  
}


void PlaneEditor::render(QPainter &p) {
  Group const &here = ed->currentGroup();
  Object const &obj = here.object(d->hovernode);
  if (obj.isPlane()) {
    FilledPlane const &fp(obj.asPlane());
    QPolygonF peri = fp.perimeter.toMils();
    QPainterPath clip;
    clip.addPolygon(peri);
    p.setClipPath(clip);
    // First, render all the corners
    p.setPen(Qt::NoPen);
    p.setBrush(QColor(255, 255, 255, 128));
    double rad = d->mrg.toMils();
    for (QPointF pt: peri)
      p.drawEllipse(pt, rad, rad);
      
    qDebug() << "Hovering on plane";
  } else {
    qDebug() << "Not on plane";
  }
}
