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
  }
  Point constrain(Point p) {
    Dim grid = ed->layout.board().grid;
    if (constr45 && tracing) {
      Point p0 = tracestart.roundedTo(grid);
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
public:
  EData *ed;
  Point tracestart;
  Point tracecurrent;
  bool tracing;
  Dim linewidth;
  Layer layer;
  bool constr45;
  bool onsomething;
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
  d->tracestart = p;
  move(p);
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

  UndoCreator uc(d->ed, true);
  Group &here(d->ed->currentGroup());
  Trace t;
  t.p1 = d->tracestart;
  t.p2 = d->tracecurrent;
  t.width = d->ed->props.linewidth;
  t.layer = d->ed->props.layer;
  here.insert(Object(t));
  if (d->onsomething)
    end();
  else
    start(d->tracecurrent);
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
  NodeID n = d->ed->currentGroup().nodeAt(p, d->ed->pressMargin(),
					  d->layer);
  LayerPoint lp = n.location(d->ed->currentGroup(),
			     p.roundedTo(d->ed->layout.board().grid));
  d->onsomething = lp.layer != Layer::Invalid;
  if (d->onsomething) 
    d->tracecurrent = lp.point;
  else
    d->tracecurrent = d->constrain(p);
}

void Tracer::render(QPainter &p) {
  if (!d->tracing)
    return;
  p.setPen(QPen(layerColor(d->layer), d->linewidth.toMils(),
		Qt::SolidLine, Qt::RoundCap));
  p.drawLine(d->tracestart.toMils(), d->tracecurrent.toMils());
}
