// Tracer.cpp

#include "Tracer.h"
#include "EData.h"
#include "UndoCreator.h"

class TRData {
public:
  TRData(EData *ed): ed(ed) {
    tracing = false;
    linewidth = ed->props.linewidth;
    layer = ed->props.layer;
    constr45 = ed->props.angleconstraint;
    onsomething = false;
    second = false;
  }
  Point toGrid(Point p) const {
    Dim grid = ed->layout.board().grid;
    return p.roundedTo(grid);
  }
  Point constrain(Point p) const {
    Dim grid = ed->layout.board().grid;
    if (constr45 && tracing) {
      Point p0 = tracestart;//.roundedTo(grid);
      p = p.roundedTo(grid);
      Dim dx = p.x - p0.x;
      Dim dy = p.y - p0.y;
      Dim dr = (abs(dx) + abs(dy))/2;
      Point p1 = p0 + Point(dx, Dim());
      Point p2 = p0 + Point(Dim(), dy);
      Point p3 = (p0 + Point(dr*sign(dx), dr*sign(dy))).roundedTo(grid);
      Point p12 = p.distance(p1) < p.distance(p2) ? p1 : p2;
      return (p.distance(p3) < p.distance(p12)) ? p3 : p12;
    } else {
      return p.roundedTo(grid);
    }
  }
  void maybeSplit(bool createUndo) {
    if (!onsomething)
      return;
    if (onnode.size() != 1)
      return; // do not enter groups
    Object const &obj(ed->currentGroup().object(onnode));
    if (obj.isTrace()) {
      Trace const &t(obj.asTrace());
      if (!t.onP1(tracecurrent, ed->pressMargin())
          && !t.onP2(tracecurrent, ed->pressMargin())) {
        if (createUndo) 
          UndoCreator uc(ed, true);
        Group &here(ed->currentGroup());
        Trace &t0(ed->currentGroup().object(onnode).asTrace());
        Trace t1 = t0;
        t0.p2 = onlp.point;
        t1.p1 = onlp.point;
        here.insert(Object(t1));
      }
    }
  }
  void insertTrace(Point from, Point to) {
    UndoCreator uc(ed, true);
    Group &here(ed->currentGroup());
    Trace t;
    t.p1 = from;
    t.p2 = to;
    t.width = ed->props.linewidth;
    t.layer = ed->props.layer;
    here.insert(Object(t));
  }

public:
  EData *ed;
  Point tracestart;
  Point tracecurrent;
  bool tracing;
  Dim linewidth;
  Layer layer;
  bool constr45;
  bool onsomething;
  NodeID onnode; // only valid if onsomething
  LayerPoint onlp; // only valid if onsomething
  bool second;
  Point previousstart;
};

void Tracer::setLayer(Layer const &l) {
  d->layer = l;
}

void Tracer::setLinewidth(Dim const &lw) {
  d->linewidth = lw;
}

Tracer::Tracer(class EData *ed): d(new TRData(ed)) {
}

Tracer::~Tracer() {
  delete d;
}

void Tracer::start(class Point const &p) {
  move(p);
  d->tracestart = d->tracecurrent;
  qDebug() << "start" << d->tracestart << d->tracecurrent;
  if (!d->tracing && d->onsomething) {
    d->maybeSplit(true);
  }
  d->tracing = true;
}

void Tracer::click(Point const &p) {
  move(p);
  if (d->tracing)
    confirm();
  else 
    start(d->tracecurrent);
}

void Tracer::confirm() {
  if (d->tracecurrent.distance(d->tracestart) < d->ed->pressMargin()) {
    qDebug() << "abort tracing due to short segment";
    end();
    return;
  }

  if (d->tracing && d->constr45 && d->second
      && d->toGrid(d->tracecurrent) == d->tracecurrent
      && d->toGrid(d->tracestart) != d->tracestart) {
    // consider undoing previous trace
    Dim lx = d->tracecurrent.x - d->tracestart.x;
    Dim ly = d->tracecurrent.y - d->tracestart.y;
    Point p0 = d->tracestart.roundedTo(d->ed->layout.board().grid);
    Dim dx = p0.x - d->tracestart.x;
    Dim dy = p0.y - d->tracestart.y;
    Point p1 = (abs(dx) < abs(dy)) 
      // consider shifting x
      ? Point(d->tracecurrent.x - sign(lx)*abs(ly), d->tracestart.y)
      // consider shifting y
      : Point(d->tracestart.x, d->tracecurrent.y - sign(ly)*abs(lx));
    d->ed->ed->undo();
    d->insertTrace(d->previousstart, p1);
    d->tracestart = p1; // fake it for now
  }
  d->insertTrace(d->tracestart, d->tracecurrent);

  if (d->onsomething) {
    d->maybeSplit(false);
    end();
  } else {
    d->previousstart = d->tracestart;
    d->second = true;
    start(d->tracecurrent);
  }
}

void Tracer::end() {
  d->tracing = false;
}

bool Tracer::isTracing() const {
  return d->tracing;
}

void Tracer::pickup(Point const &p) {
  int id = d->ed->visibleObjectAt(p, d->ed->pressMargin());
  if (id<0) {
    end();
    return;
  }
  Trace t;
  
  { Object const &obj(d->ed->currentGroup().object(id));
    if (obj.isTrace()) {
      t = obj.asTrace();
      d->ed->props.linewidth = t.width;
      d->ed->props.layer = t.layer;
    } else {
      end();
      return;
    }
  }

  UndoCreator uc(d->ed, true);
  d->ed->currentGroup().remove(id);

  setLayer(t.layer);
  setLinewidth(t.width);
  if (p.distance(t.p1)<p.distance(t.p2)) 
    // pickup p1, leave p2
    start(t.p2);
  else 
    // pickup p2
    start(t.p1);
  move(p);
}

void Tracer::move(Point const &p) {
  d->onnode = d->ed->currentGroup().nodeAt(p, d->ed->pressMargin(), d->layer);
  d->onlp = d->onnode.location(d->ed->currentGroup(), p,
                               d->ed->pressMargin());
                     //p.roundedTo(d->ed->layout.board().grid));
  d->onsomething = d->onlp.layer != Layer::Invalid;
  if (d->onsomething) 
    d->tracecurrent = d->onlp.point;
  else
    d->tracecurrent = d->constrain(p);
  qDebug() << "Tracer::move" << p << d->onnode
           << d->onlp.layer << d->onlp.point 
           << d->onsomething << d->tracecurrent;
}

void Tracer::render(QPainter &p) {
  if (!d->tracing)
    return;
  p.setPen(QPen(layerColor(d->layer), d->linewidth.toMils(),
		Qt::SolidLine, Qt::RoundCap));
  p.drawLine(d->tracestart.toMils(), d->tracecurrent.toMils());
}
