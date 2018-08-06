// ORenderer.cpp

#include "ORenderer.h"
#include "data/SimpleFont.h"
#include "data/PCBNet.h"

#include <QSvgGenerator>
#include <QBuffer>

constexpr double inNetMils = 10;

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

void ORenderer::setStuckPoints(QMap<Layer, QSet<Point> > const &pp) {
  stuckpts = pp;
}

void ORenderer::setPurePoints(QMap<Layer, QSet<Point> > const &pp) {
  purepts = pp;
}

void ORenderer::setSelPoints(QMap<Layer, QSet<Point> > const &pp) {
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

void ORenderer::drawTrace(Trace const &t, bool selected, bool innet) {
  if (t.layer != layer)
    return;
  
  Point p1 = origin + t.p1;
  Point p2 = origin + t.p2;
  if (toplevel) {
    if (selected) {
      if (!stuckpts[layer].contains(p1))
	p1 += movingdelta;
      if (!stuckpts[layer].contains(p2))
	p2 += movingdelta;
    } else {
      if ((selpts[layer].contains(p1) || purepts[layer].contains(p1))
	  && !stuckpts[layer].contains(p1))
	p1 += movingdelta;
      if ((selpts[layer].contains(t.p2) || purepts[layer].contains(t.p2))
	  && !stuckpts[layer].contains(p2))
	p2 += movingdelta;
    }
  }
  if (innet) {
    p->setPen(QPen(layerColor(t.layer, selected), t.width.toMils() + inNetMils,
		   Qt::SolidLine, Qt::RoundCap));
    p->drawLine(p1.toMils(), p2.toMils());
  }
  p->setPen(QPen(layerColor(t.layer, selected), t.width.toMils(),
		Qt::SolidLine, Qt::RoundCap));
  p->drawLine(p1.toMils(), p2.toMils());
}

void ORenderer::drawHole(Hole const &t, bool selected, bool innet) {
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
    if (innet) {
      p->setBrush(layerColor(layer, selected));
      if (t.square)
	p->drawRect(QRectF(p1.toMils()
			   - QPointF(od/2+inNetMils/2, od/2+inNetMils/2),
			   QSizeF(od+inNetMils, od+inNetMils)));
      else 
	p->drawEllipse(p1.toMils(), od/2+inNetMils/2, od/2+inNetMils/2);
    }
    p->setBrush(layerColor(layer, selected));
    if (t.square)
      p->drawRect(QRectF(p1.toMils() - QPointF(od/2, od/2), QSizeF(od, od)));
    else 
      p->drawEllipse(p1.toMils(), od/2, od/2);
  }
}

void ORenderer::drawPad(Pad const &t, bool selected, bool innet) {
  if (t.layer != layer)
    return;
  
  p->setPen(Qt::NoPen);

  QPointF p0 = t.p.toMils();
  if (toplevel && selected)
    p0 += movingdelta.toMils();
  
  double w = t.width.toMils();
  double h = t.height.toMils();
  if (innet) {
    p->setBrush(layerColor(layer, selected));
    QPointF dp(w+inNetMils, h+inNetMils);
    p->drawRect(QRectF(p0 - dp/2, p0 + dp/2));
  }
  p->setBrush(layerColor(layer, selected));
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
  // For Painter, 0 = right, 16*90 = top, etc.
  int start_ang;
  int span_ang;
  if (t.angle<0) {
    start_ang = 90 + t.angle;
    span_ang = -t.angle;
  } else {
    start_ang = 90 - t.angle/2;
    span_ang = t.angle;
  }
  start_ang -= 90 * (t.rot&3);
  p->drawArc(rect, 16*start_ang, 16*span_ang);
}

void ORenderer::drawGroup(Group const &g, bool selected, PCBNet const &net) {
  Point ori;
  if (selected && toplevel)
    ori += movingdelta;
  pushOrigin(ori);
  for (int id: g.keys()) {
    PCBNet subnet;
    for (NodeID nid: net) 
      if (!nid.isEmpty() && nid.first()==id)
	subnet << nid.tail();
    drawObject(g.object(id), selected, subnet);
  }
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

void ORenderer::drawObject(Object const &o, bool selected,
			   PCBNet const &subnet) {
  /* In toplevel *only*, and only during moves, selected objects are
     translated by the movingdelta and nonselected traces with
     selected endpoints have those endpoints translated.
   */
  switch (o.type()) {
  case Object::Type::Trace:
    drawTrace(o.asTrace(), selected, !subnet.isEmpty());
    break;
  case Object::Type::Text: 
    drawText(o.asText(), selected);
   break;
  case Object::Type::Hole:
    drawHole(o.asHole(), selected, !subnet.isEmpty());
    break;
  case Object::Type::Pad: 
    drawPad(o.asPad(), selected, !subnet.isEmpty());
    break;
  case Object::Type::Arc: 
    drawArc(o.asArc(), selected);
    break;
  case Object::Type::Group:
    drawGroup(o.asGroup(), selected, subnet);
    break;
  default:
    break;
  }
}

QByteArray ORenderer::objectToSvg(Object const &obj,
				  Dim margin, Dim minSize) {
  Rect br = obj.boundingRect();
  qDebug() << "obj bbox" << br;
  br.grow(margin);
  Dim dx = minSize - br.width;
  if (dx.isPositive()) {
    br.left -= dx/2;
    br.width += dx;
  }
  Dim dy = minSize - br.height;
  if (dy.isPositive()) {
    br.top -= dy/2;
    br.height += dy;
  }

  qDebug() << "svg bbox" << br;

  QBuffer output;
  output.open(QIODevice::WriteOnly);
  { QSvgGenerator gen;
    gen.setOutputDevice(&output);
    gen.setTitle("PCB Component");
    gen.setDescription("Generated by CPCB");
    QRectF rr = br.toMils();
    gen.setSize(rr.size().toSize());
    gen.setViewBox(QRectF(QPointF(0,0), rr.size()));
    gen.setResolution(1000);
    
    { QPainter ptr(&gen);
      ptr.translate(-rr.topLeft());
      ptr.setBrush(QBrush(QColor(0,0,0)));
      ptr.setPen(QPen(Qt::NoPen));
      ptr.drawRect(rr);
      render(obj, &ptr);
    }
  }
  output.close();
  return output.data();
}

void ORenderer::render(Object const &obj, QPainter *ptr) {
  ORenderer rndr(ptr);
  rndr.setLayer(Layer::Bottom);
  rndr.drawObject(obj);
  rndr.setLayer(Layer::Top);
  rndr.drawObject(obj);
  rndr.setLayer(Layer::Invalid);
  rndr.drawObject(obj);
  rndr.setLayer(Layer::Silk);
  rndr.drawObject(obj);
}

void ORenderer::render(Group const &grp, QPainter *ptr) {
  ORenderer rndr(ptr);
  rndr.setLayer(Layer::Bottom);
  rndr.drawGroup(grp);
  rndr.setLayer(Layer::Top);
  rndr.drawGroup(grp);
  rndr.setLayer(Layer::Invalid);
  rndr.drawGroup(grp);
  rndr.setLayer(Layer::Silk);
  rndr.drawGroup(grp);
}
