// ORenderer.cpp

#include "ORenderer.h"
#include "data/SimpleFont.h"
#include "data/PCBNet.h"
#include "data/FilledPlane.h"

#include <QSvgGenerator>
#include <QBuffer>

constexpr double inNetMils = 10;
constexpr double overrideMils = 30;

ORenderer::ORenderer(QPainter *p, Point const &o): p(p), origin(o) {
  toplevel = true;
  overr = Override::None;
  subl = Sublayer::Main;
}

void ORenderer::setPainter(QPainter *p1) {
  p = p1;
}

ORenderer::~ORenderer() {
}

void ORenderer::setBoard(Board const &b) {
  brd = b;
}

void ORenderer::setOverride(Override ovr) {
  overr = ovr;
}

void ORenderer::setLayer(Layer l, ORenderer::Sublayer s) {
  layer = l;
  subl = s;
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

QColor ORenderer::overrideColor(QColor const &c) const {
  switch (overr) {
  case Override::None:
    return c;
  case Override::WronglyIn:
    return QColor(255, 100, 255);
  case Override::Missing:
    return QColor(20, 200, 255);
  default:
    return c;
  }
}

void ORenderer::drawPlane(FilledPlane const &t, bool selected, bool innet) {
  if (t.layer != layer)
    return;
  if (subl != Sublayer::Plane)
    return;
  p->setBrush(brushColor(selected, innet));
  p->drawPolygon(t.perimeter.toMils());
}

void ORenderer::drawTrace(Trace const &t, bool selected, bool innet) {
  if (t.layer != layer)
    return;
  if (subl == Sublayer::Plane)
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
  p->setPen(QPen(brushColor(selected, innet), t.width.toMils()
                 + extraMils(innet, t.width),
                 Qt::SolidLine, Qt::RoundCap));
  p->drawLine(p1.toMils(), p2.toMils());
}

double ORenderer::extraMils(bool innet, Dim lw) const {
  if (subl==Sublayer::Clearance) 
    return brd.traceClearance(lw).toMils() * 2;
  else if (innet) 
    return overr==Override::None ? inNetMils : overrideMils;
  else
    return 0;
}  

double ORenderer::extraMils(bool innet, Dim w, Dim h) const {
  if (subl==Sublayer::Clearance) 
    return brd.padClearance(w, h).toMils() * 2;
  else if (innet) 
    return overr==Override::None ? inNetMils : overrideMils;
  else
    return 0;
}  

QColor ORenderer::brushColor(bool selected, bool innet) const {
  if (subl==Sublayer::Clearance)
    return QColor(0, 0, 0, 0);
  else if (innet) 
    return overrideColor(layerColor(layer, selected));
  else
    return layerColor(layer, selected);
}

void ORenderer::drawHole(Hole const &t, bool selected, bool innet) {
  if (subl == Sublayer::Plane)
    return;
  if (subl == Sublayer::Clearance && t.noclear)
    return;
  bool inv = layer==Layer::Invalid; // this is used for drilling
  bool tb = layer==Layer::Bottom || layer==Layer::Top;
  if (overr != Override::None) {
    // easiest way to enforce override is to simply tweak the flags
    // elegant? no. effective? yes.
    inv = false; tb = true;
    innet = true; selected = false;
  }
    
  if (!inv && !tb)
    return;
  
  Point p1 = origin + t.p;
  if (selected && toplevel) 
    p1 += movingdelta;
  p->setPen(QPen(Qt::NoPen));
  QPointF p0 = p1.toMils();
  if (inv) {
    double id = t.id.toMils();
    p->setBrush(QColor(0,0,0));
    p->drawEllipse(p0, id/2, id/2);
  } else {
    double od = t.od.toMils();
    double extramils = extraMils(innet, t.od, t.od);
    p->setBrush(brushColor(selected, innet));
    if (t.square)
      p->drawRect(QRectF(p0
                         - QPointF(od/2+extramils/2, od/2+extramils/2),
                         QSizeF(od+extramils, od+extramils)));
    else 
      p->drawEllipse(p0, od/2+extramils/2, od/2+extramils/2);
  }
  if (subl==Sublayer::Main && t.fpcon==layer) {
    double dxm = (t.od/2 + brd.padClearance(t.od, t.od)).toMils();
    p->setPen(QPen(brushColor(selected, innet),
                   brd.fpConWidth(t.od, t.od).toMils(),
                   Qt::SolidLine, Qt::RoundCap));
    p->drawLine(p0 - QPointF(-dxm, 0), p0 + QPointF(dxm, 0));
    p->drawLine(p0 - QPointF(0, -dxm), p0 + QPointF(0, dxm));
  }
}

void ORenderer::drawPad(Pad const &t, bool selected, bool innet) {
  if (subl == Sublayer::Plane)
    return;
  if (subl == Sublayer::Clearance && t.noclear)
    return;
  if (overr == Override::None) {
    if (t.layer != layer)
      return;
  } else {
    selected = false;
    innet = true;
  }
  
  p->setPen(Qt::NoPen);

  QPointF p0 = t.p.toMils();
  if (toplevel && selected)
    p0 += movingdelta.toMils();
  
  double w = t.width.toMils();
  double h = t.height.toMils();

  double extramils = extraMils(innet, t.width, t.height);
  p->setBrush(brushColor(selected, innet));
  QPointF dp(w+extramils, h+extramils);
  p->drawRect(QRectF(p0 - dp/2, p0 + dp/2));

  if (subl==Sublayer::Main && t.fpcon) {
    Dim pc = brd.padClearance(t.width, t.height);
    double dxm = w/2 + pc.toMils();
    double dym = h/2 + pc.toMils();
    p->setPen(QPen(brushColor(selected, innet),
                   brd.fpConWidth(t.width, t.height).toMils(),
                   Qt::SolidLine, Qt::RoundCap));
    p->drawLine(p0 - QPointF(-dxm, 0), p0 + QPointF(dxm, 0));
    p->drawLine(p0 - QPointF(0, -dym), p0 + QPointF(0, dym));
  }
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

void ORenderer::drawGroup(Group const &g, bool selected,
			  QSet<NodeID> const &net) {
  Point ori;
  if (selected && toplevel)
    ori += movingdelta;
  pushOrigin(ori);
  for (int id: g.keys()) {
    QSet<NodeID> subnet;
    for (NodeID const &nid: net) 
      if (!nid.isEmpty() && nid.first()==id)
	subnet << nid.tail();
    drawObject(g.object(id), selected, subnet);
  }
  popOrigin();
}

void ORenderer::drawText(Text const &t, bool selected) {
  if (t.layer!=layer)
    return;
  
  if (subl!=Sublayer::Main)
    return; // actually, we should draw a rectangle in Clearance...
  
  SimpleFont const &sf(SimpleFont::instance());
  double scl = sf.scaleFactor(t.fontsize);
  Point pt = origin + t.p;
  if (selected && toplevel)
    pt += movingdelta;
  p->save();
  p->setPen(QPen(QColor(255,255,255), sf.lineWidth().toMils()));
  p->translate(pt.toMils());

  //  p->setPen(QPen(QColor(255,255,255), 2));
  //  p->drawEllipse(QPointF(0,0), 5,5);

  p->rotate(90*t.orient.rot);

  int xflip = ((t.layer==Layer::Bottom) ^ t.orient.flip) ? -1 : 1;
  p->scale(xflip*scl, -scl);
  p->setPen(QPen(layerColor(t.layer, selected), sf.lineWidth().toMils(),
		Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
  for (int k=0; k<t.text.size(); k++) {
    QVector<Polyline> const &glyph = sf.character(t.text[k].unicode());
    for (auto const &pp0: glyph) {
      QPolygonF pp;
      for (Point const &p0: pp0)
	pp << p0.toMils();
      if (pp.size()==1)
	p->drawPoint(pp[0]);
      else
	p->drawPolyline(pp);
    }
    p->translate(sf.dx().toMils(), 0);
  }
  p->restore();
}

void ORenderer::drawObject(Object const &o, bool selected,
			   QSet<NodeID> const &subnet) {
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
  case Object::Type::Plane:
    drawPlane(o.asPlane(), selected, !subnet.isEmpty());
  default:
    break;
  }
}

QByteArray ORenderer::objectToSvg(Object const &obj,
				  Dim margin, Dim minSize) {
  Rect br = obj.boundingRect();
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
