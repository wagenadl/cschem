// HoverPin.cpp

#include "HoverPin.h"
#include <QBrush>
#include <QPen>
#include "Scene.h"
#include "SceneElement.h"
#include <QDebug>

HoverPin::HoverPin(class Scene *scene): scene(scene) {
  setBrush(QBrush(QColor(128, 128, 128, 0))); // initially invisible
  setPen(QPen(Qt::NoPen));
  setZValue(-10);
}

static double L2(QPointF p) {
  return p.x()*p.x() + p.y()*p.y();
}

void HoverPin::updateHover(QPointF p, int elt1, bool allowJunction) {
  auto const &elts = scene->elements();
  auto const &lib = scene->library();
  auto const &circ = scene->circuit();
  
  if (elt1<0) {
    for (auto e: elts) {
      if (e->boundingRect().contains(e->mapFromScene(p))) {
        elt1 = e->id();
        break;
      }
    }
  }
  QString pin1;
  double d1 = 1e9;
  double r = lib->scale();
  QPointF pin1pos;
  
  if (elts.contains(elt1)) {
    QString sym = circ->element(elt1).symbol();
    if (sym == "junction" && !allowJunction) {
      elt1 = -1; // don't hover pin over junctions
    } else {
      Part const &part = lib->part(sym);
      for (auto pn: part.pinNames()) {
	QPointF pp = scene->pinPosition(elt1, pn);
	double d = L2(pp - p);
	if (d<d1) {
	  d1 = d;
	  pin1 = pn;
	  pin1pos = pp;
	}
      }
      if (d1 > r*r)
	elt1 = -1;
    }
  } else {
    elt1 = -1;
  }
  
  if (elt1 != elt || pin1 != pin) {
    if (elt1>0) {
      setRect(QRectF(pin1pos - QPointF(r, r), 2 * QSizeF(r, r)));
      setBrush(QColor(0, 255, 128, 255));
    } else {
      setBrush(QColor(128, 128, 128, 0));
    }
    elt = elt1;
    pin = pin1;
  }
}
