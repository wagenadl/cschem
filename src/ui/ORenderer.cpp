// ORenderer.cpp

#include "ORenderer.h"
#include "data/SimpleFont.h"

ORenderer::ORenderer(QPainter *p, Point const &o): p(p), origin(o) {
  toplevel = true;
}

ORenderer::~ORenderer() {
}

void ORenderer::setLayer(Layer l) {
  layer = l;
}

void ORenderer::setMoving(Point const &p) {
  movingdelta = p;
}

void ORenderer::setStuckPoints(QSet<Point> const &pp) {
  stuckpts = pp;
}

void ORenderer::setPurePoints(QSet<Point> const &pp) {
  purepts = pp;
}

void ORenderer::setSelPoints(QSet<Point> const &pp) {
  selpts = pp;
}

void ORenderer::pushOrigin(Point const &o1) {
  originstack << origin;
  origin += o1;
  toplevel = false;
}

void ORenderer::popOrigin() {
  origin = originstack.takeLast();
  toplevel = originstack.isEmpty();
}

void ORenderer::drawTrace(Trace const &t, bool selected) {
  if (t.layer != layer)
    return;
  
  p->setPen(QPen(layerColor(t.layer, selected), t.width.toMils(),
		Qt::SolidLine, Qt::RoundCap));
  Point p1 = origin + t.p1;
  Point p2 = origin + t.p2;
  if (toplevel) {
    if (selected) {
      if (!stuckpts.contains(p1))
	p1 += movingdelta;
      if (!stuckpts.contains(p2))
	p2 += movingdelta;
    } else {
      if ((selpts.contains(p1) || purepts.contains(p1))
	  && !stuckpts.contains(p1))
	p1 += movingdelta;
      if ((selpts.contains(t.p2) || purepts.contains(t.p2))
	  && !stuckpts.contains(p2))
	p2 += movingdelta;
    }
  }
  p->drawLine(p1.toMils(), p2.toMils());
}

void ORenderer::drawHole(Hole const &t, bool selected) {
  bool inv = layer==Layer::Invalid;
  bool tb = layer==Layer::Bottom || layer==Layer::Top;
  if (!inv && !tb)
    return;
  
  Point p1 = origin + t.p;
  if (selected && toplevel) 
    p1 += movingdelta;
  p->setPen(QPen(Qt::NoPen));
  if (inv) {
    double id = t.id.toMils();
    p->setBrush(QColor(0,0,0));
    p->drawEllipse(p1.toMils(), id/2, id/2);
  } else {
    double od = t.od.toMils();
    p->setBrush(layerColor(layer, selected));
    if (t.square)
      p->drawRect(QRectF(p1.toMils() - QPointF(od/2, od/2), QSizeF(od, od)));
    else 
      p->drawEllipse(p1.toMils(), od/2, od/2);
  }
}

void ORenderer::drawPad(Pad const &t, bool selected) {
  if (t.layer != layer)
    return;
  
  p->setPen(Qt::NoPen);
  p->setBrush(layerColor(layer, selected));

  QPointF p0 = t.p.toMils();
  if (toplevel && selected)
    p0 += movingdelta.toMils();
  
  double w = t.width.toMils();
  double h = t.height.toMils();
  p->drawRect(QRectF(p0 - QPointF(w/2,h/2), p0 + QPointF(w/2,h/2)));
}

void ORenderer::drawArc(Arc const &t, bool selected) {
  if (t.layer != layer)
    return;
  
  Point c = origin + t.center;
  if (toplevel && selected)
    c += movingdelta;
  
  p->setPen(QPen(layerColor(layer, selected), t.linewidth.toMils(),
		 Qt::SolidLine, Qt::RoundCap));
  p->setBrush(Qt::NoBrush);
  double r = t.radius.toMils();
  QRectF rect(c.toMils() - QPointF(r,r), c.toMils() + QPointF(r,r));
  switch (t.extent) {
  case Arc::Extent::Invalid: break;
  case Arc::Extent::Full: p->drawArc(rect, 0, 16*360); break;
  case Arc::Extent::LeftHalf: p->drawArc(rect, 16*90, 16*180); break;
  case Arc::Extent::RightHalf: p->drawArc(rect, 16*-90, 16*180); break;;
  case Arc::Extent::TopHalf: p->drawArc(rect, 0, 16*180); break;
  case Arc::Extent::BottomHalf: p->drawArc(rect, -16*180, 16*180); break;
  case Arc::Extent::TLQuadrant: p->drawArc(rect, 16*90, 16*90); break;
  case Arc::Extent::TRQuadrant: p->drawArc(rect, 0, 16*90); break;
  case Arc::Extent::BRQuadrant: p->drawArc(rect, -16*90, 16*90); break;
  case Arc::Extent::BLQuadrant: p->drawArc(rect, 16*180, 16*90); break;
  }
}

void ORenderer::drawGroup(Group const &g, bool selected) {
  Point ori = g.origin;
  if (selected && toplevel)
    ori += movingdelta;
  pushOrigin(ori);
  for (int id: g.keys())
    drawObject(g.object(id), selected);
  popOrigin();
}

void ORenderer::drawText(Text const &t, bool selected) {
  if (t.layer != layer)
    return;
  
  SimpleFont const &sf(SimpleFont::instance());
  Point pt = origin + t.p;
  if (selected && toplevel)
    pt += movingdelta;
  p->save();
  p->setPen(QPen(QColor(255,255,255), 4));
  p->translate(pt.toMils());
  p->rotate(90*t.orient.rot);
  int xflip = ((t.layer==Layer::Bottom) ^ t.orient.flip) ? -1 : 1;
  p->scale(xflip*t.fontsize.toMils()/sf.baseSize(),
	  -t.fontsize.toMils()/sf.baseSize());
  p->setPen(QPen(layerColor(t.layer, selected), 2,
		Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
  for (int k=0; k<t.text.size(); k++) {
    QVector<QPolygonF> const &glyph = sf.character(t.text[k].unicode());
    for (auto const &pp: glyph) 
      if (pp.size()==1)
	p->drawPoint(pp[0]);
      else
	p->drawPolyline(pp);
    p->translate(sf.dx(),0);
  }
  p->restore();
}

void ORenderer::drawObject(Object const &o, bool selected) {
  /* In toplevel *only*, and only during moves, selected objects are
     translated by the movingdelta and nonselected traces with
     selected endpoints have those endpoints translated.
   */
  switch (o.type()) {
  case Object::Type::Trace:
    drawTrace(o.asTrace(), selected);
    break;
  case Object::Type::Text: 
    drawText(o.asText(), selected);
   break;
  case Object::Type::Hole:
    drawHole(o.asHole(), selected);
    break;
  case Object::Type::Pad: 
    drawPad(o.asPad(), selected);
    break;
  case Object::Type::Arc: 
    drawArc(o.asArc(), selected);
    break;
  case Object::Type::Group:
    drawGroup(o.asGroup(), selected);
    break;
  default:
    break;
  }
}

