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
    hoverptidx = -1;
    hoveredgeidx = -1;
    moving = false;
    creating = false;
    shiftheld = false;
  }
public:
  EData *ed;
  Point hoverpt;
  NodeID hovernode;
  Dim mrg;
  Point presspt;
  int hoverptidx;
  int hoveredgeidx;
  bool moving;
  bool creating;
  Polyline premovepoly;
  Point premovept;
  bool shiftheld;
  bool movingedge;
public:
  int onVertex(Point p) const;
  int onEdge(Point p, Point *p_out) const;
  void updateHoverPtIdx();
  void updateHoverEdgeIdx();
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
  if (d->hoveredgeidx>=0)
    return; // not doing anything
  Group const &here(ed->currentGroup());
  Object const &obj(here.object(d->hovernode));
  if (obj.isPlane()) {
    if (d->hoverptidx>=0 && obj.asPlane().perimeter.size()<=3)
      return;
    // are there other ways in which we might get in trouble?
    // yes, we could be creating very thin needles. But I am going to allow
    // that for now.
    UndoCreator(ed, true);
    Group &grp(ed->currentGroup().parentOf(d->hovernode));
    if (d->hoverptidx>=0) {
      grp.object(d->hovernode.last()).asPlane().perimeter.remove(d->hoverptidx);
    } else {
      grp.remove(d->hovernode.last());
      d->hovernode = NodeID();
    }
    ed->ed->update();
  }
}

void PEData::updateHoverPtIdx() {
  hoverptidx = onVertex(hoverpt);
}

void PEData::updateHoverEdgeIdx() {
  hoveredgeidx = hoverptidx>=0
    ? -1
    : onEdge(hoverpt.roundedTo(ed->layout.board().grid), &hoverpt);
}

int PEData::onVertex(Point p) const {
  Group const &here = ed->currentGroup();
  Object const &obj = here.object(hovernode);
  if (!obj.isPlane())
    return -1;
  
  FilledPlane const &fp(obj.asPlane());
  int N = fp.perimeter.size();
  int nbest = -1;
  Dim dist = Dim::infinity();
  for (int n=0; n<N; n++) {
    Dim dist1 = p.distance(fp.perimeter[n]);
    if (dist1 < dist) {
      dist = dist1;
      nbest = n;
    }
  }
  if (dist < mrg) 
    return nbest;
  else
    return -1;
}
  
int PEData::onEdge(Point p, Point *p_out) const {
  Group const &here = ed->currentGroup();
  Object const &obj = here.object(hovernode);
  if (!obj.isPlane())
    return -1;
  
  FilledPlane const &fp(obj.asPlane());
  int N = fp.perimeter.size();
  for (int n=0; n<N; n++) {
    Segment s(fp.perimeter[n], fp.perimeter[(n+1)%N]);
    if (s.betweenEndpoints(p, mrg)) {
      if (p_out)
        *p_out = s.projectionOntoSegment(p);
      return n;
    }
  }
  return -1;
}

void PlaneEditor::mousePress(Point p, Qt::MouseButton b,
                             Qt::KeyboardModifiers m) {
  mouseMove(p, b, m);
  if (d->moving || d->creating)
    return;
  if (d->hovernode.isEmpty()) {
    // not on a node => create a new plane
    d->creating = true;
    p = p.roundedTo(ed->layout.board().grid);
    d->presspt = p;
    FilledPlane fp;
    fp.perimeter << p << p << p << p;
    fp.layer = ed->props.layer;
    d->hovernode = NodeID();
    d->hovernode << ed->currentGroup().insert(Object(fp));
  } else {
    d->updateHoverPtIdx();
    d->updateHoverEdgeIdx();
    if (d->hoverptidx>=0) {
      d->moving = true;
      d->movingedge = false;
      d->presspt = p;
      d->premovepoly
        = ed->currentGroup().object(d->hovernode).asPlane().perimeter;
      d->premovept = d->premovepoly[d->hoverptidx];
    } else if (d->hoveredgeidx>=0) {
      d->moving = true;
      d->movingedge = d->shiftheld;
      d->presspt = p;
      d->premovepoly
        = ed->currentGroup().object(d->hovernode).asPlane().perimeter;
      if (!d->movingedge) {
	ed->currentGroup().object(d->hovernode).asPlane().perimeter
	  .insert(d->hoveredgeidx+1, d->hoverpt);
	d->hoverptidx = d->hoveredgeidx + 1;
      }
      d->premovept = d->hoverpt;
    }
  }
}

void PlaneEditor::mouseRelease(Point p,
                               Qt::MouseButton b,
                               Qt::KeyboardModifiers m) {
  mouseMove(p, b, m);
  if (d->moving) {
    Polyline postmovepoly
      = ed->currentGroup().object(d->hovernode).asPlane().perimeter;
    Point postmovept = postmovepoly[d->hoverptidx];
    // first, restore...
    ed->currentGroup().object(d->hovernode).asPlane().perimeter
      = d->premovepoly;
    if (postmovept != d->premovept) {
      // then create undo point...
      UndoCreator uc(ed, true);
      // finally remake
      ed->currentGroup().object(d->hovernode).asPlane().perimeter
        = postmovepoly;
    }
    d->moving = false;
    ed->ed->update();
  } else if (d->creating) {
    FilledPlane fp = ed->currentGroup().object(d->hovernode).asPlane();
    Polyline postmovepoly = fp.perimeter;
    // first, restore
    ed->currentGroup().remove(d->hovernode[0]); // by construction, not deep
    // then:
    if (postmovepoly[2].x!=postmovepoly[0].x
        && postmovepoly[2].y!=postmovepoly[0].y) { // if nonempty
      UndoCreator uc(ed, true);
      d->hovernode = NodeID();
      d->hovernode << ed->currentGroup().insert(Object(fp));
    }
    d->creating = false;
    ed->ed->update();
  }
}

void PlaneEditor::mouseMove(Point p,
                            Qt::MouseButton,
                            Qt::KeyboardModifiers m) {
  d->shiftheld = m & Qt::ShiftModifier;
  Point p0 = d->hoverpt;
  d->hoverpt = p;
  if (d->moving) {
    Group &here = ed->currentGroup();
    Object &obj = here.object(d->hovernode);
    FilledPlane &fp(obj.asPlane());
    if (d->movingedge) {
      Point delta = p - d->presspt;
      Point p0 = d->premovepoly.vertex(d->hoveredgeidx);
      Point p1 = d->premovepoly.vertex(d->hoveredgeidx+1);
      if (p0.x==p1.x) {
	Point px = (p0+delta).roundedTo(ed->layout.board().grid);
	p1.x += px.x - p0.x;
	p0.x = px.x;
      } else if  (p0.y==p1.y) {
	Point py = (p0+delta).roundedTo(ed->layout.board().grid);
	p1.y += py.y - p0.y;
	p0.y = py.y;
      } else {
	Point p = (p0+delta).roundedTo(ed->layout.board().grid);
	p1 += p - p0;
	p0 = p;
      }
      Point p0a = fp.perimeter.vertex(d->hoveredgeidx);
      fp.perimeter.setVertex(d->hoveredgeidx, p0);
      if (fp.perimeter.acceptableMove(d->hoveredgeidx+1, p1)) 
	fp.perimeter.setVertex(d->hoveredgeidx+1, p1);
     else
       fp.perimeter.setVertex(d->hoveredgeidx, p0a);
    } else  {
      p = (p + d->premovept - d->presspt).roundedTo(ed->layout.board().grid);
      if (fp.perimeter.acceptableMove(d->hoverptidx, p))
	fp.perimeter[d->hoverptidx] = p;
    }
    ed->ed->update();
  } else if (d->creating) {
    p = p.roundedTo(ed->layout.board().grid);
    Group &here = ed->currentGroup();
    Object &obj = here.object(d->hovernode);
    FilledPlane &fp(obj.asPlane());
    fp.perimeter[2] = p;
    fp.perimeter[1].x = p.x;
    fp.perimeter[3].y = p.y;
    ed->ed->update();
  } else {
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
    int oldptidx = d->hoverptidx;
    d->updateHoverPtIdx();
    int oldedgidx = d->hoveredgeidx;
    d->updateHoverEdgeIdx();
    if (oldptidx!=d->hoverptidx || oldedgidx!=d->hoveredgeidx
        || (d->hoveredgeidx>=0 && p0!=d->hoverpt))
      ed->ed->update();
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
    ed->updateOnWhat(true); // rebuild net
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
    if (d->hoverptidx>=0) {
      p.setBrush(QColor(255, 255, 255, 128));
      p.drawEllipse(peri[d->hoverptidx], rad, rad);
    } else if (d->hoveredgeidx>=0) {
      if (d->moving ? d->movingedge : d->shiftheld) {
	QPointF p0 = peri[d->hoveredgeidx];
	QPointF p1 = peri[(d->hoveredgeidx+1) % peri.size()];
	p.setPen(QPen(QColor(255, 255, 255, 128), rad*2));
	p.drawLine(p0, p1);
      } else {
	p.setBrush(QColor(255, 255, 255, 128));
	p.drawEllipse(d->hoverpt.toMils(), rad, rad);
      }
    }
    qDebug() << "Hovering on plane";
  } else {
    qDebug() << "Not on plane";
  }
}
