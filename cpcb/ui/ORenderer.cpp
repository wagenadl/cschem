// ORenderer.cpp

#include "ORenderer.h"
#include "data/SimpleFont.h"
#include "data/PCBNet.h"
#include "data/FilledPlane.h"
#include "data/pi.h"

#include <QSvgGenerator>
#include <QBuffer>


static QColor grayPadColor() {
  return QColor(150,150,150);
}

ORenderer::ORenderer(QPainter *p, Point const &o, bool pnporient):
  p(p), origin(o), pnporient(pnporient) {
  toplevel = true;
  overr = Override::None;
  subl = Sublayer::Main;
  inNetMils = 10;
  overrideMils = 20;
}

void ORenderer::setInNetMils(double mils) {
  inNetMils = mils;
  overrideMils = 2*inNetMils;
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
  p->setPen(Qt::NoPen);
  p->setBrush(brushColor(selected, innet).darker(175));
  Polyline pp = t.perimeter;
  if (selected && toplevel) 
    pp.translate(movingdelta);
  p->drawPolygon(pp.toMils());
}

void ORenderer::drawTrace(Trace const &t, bool selected, bool innet) {
  if (t.layer != layer)
    return;
  if (subl == Sublayer::Plane)
    return;
  if (subl == Sublayer::Clearance && t.noclear)
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
  QColor c = (pnporient && (layer==Layer::Top || layer==Layer::Bottom))
    ? grayPadColor()
    : brushColor(selected, innet);
  p->setPen(QPen(c, t.width.toMils()
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
    return QColor(0,0,0, 0); //boardColor();
  else if (innet) 
    return overrideColor(layerColor(layer, selected));
  else
    return layerColor(layer, selected);
}

void ORenderer::drawHole(Hole const &hole, bool selected, bool innet) {
  if (subl == Sublayer::Plane)
    return;
  if (subl == Sublayer::Clearance && hole.noclear)
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
  
  Point p1 = origin + hole.p;
  if (selected && toplevel) 
    p1 += movingdelta;
  QPointF p0 = p1.toMils();
  double id = hole.id.toMils();
  double dx = hole.slotlength.toMils()/2;
  double od = hole.od.toMils();
  double pc = brd.padClearance(hole.od, hole.od).toMils();
  double fpover = brd.fpConOverlap().toMils();
  double extramils = extraMils(innet, hole.od, hole.od);
  double cs = hole.rota.cos();
  double sn = hole.rota.sin();

  QColor c = (pnporient 
              && !(hole.ref=="1"
                   || hole.ref.startsWith("1/")
                   || hole.ref.endsWith("/1")))
    ? grayPadColor()
    : brushColor(selected, innet);
  
  // draw fpcon
  if (subl==Sublayer::Extra && layer!=Layer::Invalid
      && hole.fpcon==layer && !pnporient) {
    c = c.darker(150);
    double dym = od/2 + pc + fpover;
    double dxm = dym + dx;
    if (hole.noclear) {
      /*
      if (dx>0) {
        p->setPen(QPen(c, od + 2*pc + 2*fpover, Qt::SolidLine, Qt::RoundCap));
        QPointF dxy(dx*cs, dx*sn);
        p->drawLine(p0 - dxy, p0 + dxy);
      } else {
        p->setBrush(c);
        p->setPen(QPen(Qt::NoPen));
        if (hole.square) {
          QPointF dp(dxm, dym);
          QRectF r(-dp, dp);
          if (hole.rota) {
            QTransform xf; xf.rotate(hole.rota);
            p->drawPolygon(xf.map(r).translated(p0));
          } else {
            p->drawRect(r.translated(p0));
          }
        } else {
          p->drawEllipse(p0, dym, dym);
        }
      }
      */
    } else {
      // draw four rays
      QPointF dmaj(cs*dxm, sn*dxm);
      QPointF dmin(-sn*dym, cs*dym);
      p->setPen(QPen(c, pc, Qt::SolidLine, Qt::FlatCap));
      p->drawLine(p0 - dmaj, p0 + dmaj);
      p->drawLine(p0 - dmin, p0 + dmin);
    }
    return;
  }

  if (inv) {
    // draw cutout
    if (dx>0) {
      p->setPen(QPen(QColor(0,0,0),
		     id, Qt::SolidLine, Qt::RoundCap));
      QPointF dxy(dx*cs, dx*sn);
      p->drawLine(p0 - dxy, p0 + dxy);
    } else {
      p->setBrush(QColor(0,0,0));
      p->setPen(Qt::NoPen);
      p->drawEllipse(p0, id/2, id/2);
    }
  } else {
    // draw surrounding pad
    if (dx>0 || hole.square) {
      p->setPen(QPen(c, od+extramils, Qt::SolidLine,
                     hole.square ? Qt::FlatCap : Qt::RoundCap));
      double dx1 = hole.square ? dx + od/2 + extramils/2 : dx;
      QPointF dxy(dx1*cs, dx1*sn);
      p->drawLine(p0 - dxy, p0 + dxy);
    } else {
      QBrush b(c);
      if (hole.via)
	b.setStyle(layer==Layer::Top ? Qt::Dense5Pattern : Qt::Dense3Pattern);
      p->setBrush(b);
      p->setPen(QPen(Qt::NoPen));
      p->drawEllipse(p0, od/2+extramils/2, od/2+extramils/2);
    }
  }
}

void ORenderer::drawNPHole(NPHole const &h, bool selected, bool /*innet*/) {
  if (layer!=Layer::Invalid)
    return;
  Point p1 = origin + h.p;
  if (selected && toplevel) 
    p1 += movingdelta;
  QPointF p0 = p1.toMils();
  double id = h.d.toMils();
  QColor col(selected ? QColor(255, 255, 255) : p->background().color());
  if (h.slotlength.isPositive()) {
    double dx = h.slotlength.toMils()/2;
    QPoint dxy(dx*h.rota.cos(), dx*h.rota.sin());
    p->setPen(QPen(col, id, Qt::SolidLine, Qt::RoundCap));
    p->drawLine(p0 - dxy, p0 + dxy);
  } else {
    p->setBrush(col);
    p->setPen(Qt::NoPen);
    p->drawEllipse(p0, id/2, id/2);
  }
  p->setPen(QPen(selected ? QColor(180, 180, 180) : QColor(120,120,120),
		 8, Qt::SolidLine, Qt::FlatCap)); // arbitrary thickness
  for (int phi=1; phi<8; phi+=2) {
    double dx = cos(phi*PI/4);
    double dy = sin(phi*PI/4);
    p->drawLine(p0 + id*QPointF(dx, dy)*.05, p0 + id*QPointF(dx, dy)*.45);
  }
}

void ORenderer::drawPad(Pad const &pad, bool selected, bool innet) {
  if (subl == Sublayer::Plane)
    return;
  if (subl == Sublayer::Clearance && pad.noclear)
    return;
  if (overr == Override::None) {
    if (pad.layer != layer)
      return;
  } else {
    selected = false;
    innet = true;
  }
  
  QPointF p0 = (origin + pad.p).toMils();
  if (toplevel && selected)
    p0 += movingdelta.toMils();
  
  double w = pad.width.toMils();
  double h = pad.height.toMils();

  QColor c = (pnporient && !(pad.ref=="1"
                             || pad.ref.startsWith("1/")
                             || pad.ref.endsWith("/1")))
    ? grayPadColor()
    : brushColor(selected, innet);
  
  if (subl==Sublayer::Extra && pad.fpcon && !pnporient) {
    if (pad.noclear)
      return;
    
    c = c.darker(150);
    Dim pc = brd.padClearance(pad.width, pad.height) + brd.fpConOverlap();
    double dxm = w/2 + pc.toMils();
    double dym = h/2 + pc.toMils();
    double cs = pad.rota.cos();
    double sn = pad.rota.sin();
    QPoint dw(dxm*cs, dxm*sn);
    QPoint dh(-dym*sn, dym*cs);
    // draw four rays
    p->setPen(QPen(c,
                   brd.fpConWidth(pad.width, pad.height).toMils(),
                   Qt::SolidLine, Qt::FlatCap));
    p->drawLine(p0 - dw, p0 + dw);
    p->drawLine(p0 - dh, p0 + dh);
    return;
  }
  
  p->setPen(Qt::NoPen);
  p->setBrush(c);
  double extramils = extraMils(innet, pad.width, pad.height);
  QPointF dp(w+extramils, h+extramils);
  QRectF r(-dp/2, dp/2);
  if (pad.rota.degrees()) {
    QTransform xf; xf.rotate(pad.rota.degrees());
    p->drawPolygon(xf.map(r).translated(p0));
  } else {
    p->drawRect(r.translated(p0));
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
  int start_ang = 90 - t.rota.degrees() - t.angle;
  int span_ang = t.angle;
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
  
  if (pnporient) {
    Rect r = g.boundingRect();
    int ori = g.nominalRotation();
    Point p0;
    switch (ori) {
    case 0:
      p0 = Point(r.left + r.width/2, r.top);
      break;
    case 90:
      p0 = Point(r.left, r.top + r.height/2);
      break;
    case 180:
      p0 = Point(r.left + r.width/2, r.top + r.height);
      break;
    case 270:
      p0 = Point(r.left + r.width, r.top + r.height/2);
      break;
    default:
      qDebug() << "Noncanonical orientation";
      return;
    }
    p->setBrush(QColor(0,255,255));
    p->setPen(Qt::NoPen);
    QPolygonF pp;
    FreeRotation rot(-ori);
    pp << (p0+Point(Dim::fromMM(-1.5), Dim::fromMM(.5)).rotatedFreely(rot)).toMils();
    pp << (p0+Point(Dim::fromMM(1.5), Dim::fromMM(.5)).rotatedFreely(rot)).toMils();
    pp << (p0+Point(Dim(), Dim::fromMM(-1.5)).rotatedFreely(rot)).toMils();
    p->drawPolygon(pp);
  }
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

  //Rect r = t.boundingRect();
  //p->drawRect(r.toMils());
  //p->drawRect(Rect(t.p - Point(Dim::fromMils(5), Dim::fromMils(5)),
  //                 t.p + Point(Dim::fromMils(5), Dim::fromMils(5))).toMils());
  
  p->translate(pt.toMils());

  //  p->setPen(QPen(QColor(255,255,255), 2));
  //  p->drawEllipse(QPointF(0,0), 5,5);

  p->rotate(t.rota.degrees());
  int xflip = ((t.layer==Layer::Bottom || t.layer==Layer::BSilk) ^ t.flip)
    ? -1 : 1;

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
  case Object::Type::NPHole:
    drawNPHole(o.asNPHole(), selected, !subnet.isEmpty());
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
				  Dim margin, Dim minSize, float scale) {
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
    gen.setSize((rr.size()*scale).toSize());
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
  for (auto l: QList<Layer>{Layer::Bottom, Layer::BSilk,
                            Layer::Top, Layer::Invalid,
                            Layer::Silk, Layer::Panel}) {
    rndr.setLayer(l);
    rndr.drawObject(obj);
  }
}

void ORenderer::render(Group const &grp, QPainter *ptr) {
  ORenderer rndr(ptr);
  for (auto l: QList<Layer>{Layer::Bottom, Layer::BSilk,
                            Layer::Top, Layer::Invalid,
                            Layer::Silk, Layer::Panel}) {
    rndr.setLayer(l);
    rndr.drawGroup(grp);
  }
}

QColor ORenderer::boardColor() {
  static QColor c(70, 30, 30);
  //  static QColor c(80, 0, 0);
  return c;
}

QColor ORenderer::backgroundColor() {
  static QColor c(0, 0, 0);
  return c;
}

