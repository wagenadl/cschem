// SceneElement.cpp

#include "SceneElement.h"
#include <QGraphicsSvgItem>
#include <QGraphicsEllipseItem>
#include <QGraphicsItemGroup>
#include "file/Element.h"
#include "Scene.h"
#include <QDebug>
#include <QGraphicsColorizeEffect>
#include <QGraphicsSceneMouseEvent>

class SceneElementData {
public:
  SceneElementData() {
    scene = 0;
    id = 0;
    element = 0;
    name = 0;
    value = 0;
    label = 0;
    dragmoved = false;
    hovering = false;
    showhover = true;
  }
public:
  void markHover() {
    if (hovering && showhover) {
      auto *ef = new QGraphicsColorizeEffect;
      ef->setColor(QColor(0, 128, 255));
      element->setGraphicsEffect(ef);
    } else {
      element->setGraphicsEffect(0);    
    }
  }
public:
  Scene *scene;
  int id;
  QGraphicsSvgItem *element;
  QGraphicsTextItem *name;
  QGraphicsTextItem *value;
  QGraphicsTextItem *label;
public:
  bool dragmoved;
  bool hovering;
  bool showhover;
};

SceneElement::SceneElement(class Scene *parent, Element const &elt) {
  d = new SceneElementData;
  //
  d->scene = parent;
  d->id = elt.id();
  QPoint pos = elt.position();
  QString sym = elt.symbol();

  PartLibrary const *lib = d->scene->library();
  Part const &part = lib->part(sym);
  if (!part.isValid())
    qDebug() << "Cannot find svg for symbol" << sym;
  double s = lib->scale();
  QPointF o = part.origin();

  QSvgRenderer *r = lib->renderer(sym);

  d->element = new QGraphicsSvgItem;
  if (r)
    d->element->setSharedRenderer(r);
  else
    qDebug() << "Cannot construct renderer for symbol" << sym;    
  d->element->setPos(-o);
  addToGroup(d->element);

  parent->addItem(this);
  setPos(s * pos);

  if (sym == "junction")
    setZValue(20);
  else
    setZValue(10);
  
  setAcceptHoverEvents(true);
  setFlag(ItemIsMovable);
  setFlag(ItemIsSelectable);
  setCacheMode(DeviceCoordinateCache);
}

SceneElement::~SceneElement() {
  delete d;
}

// static double L2(QPointF p) {
//   return p.x()*p.x() + p.y()*p.y();
// }

void SceneElement::mousePressEvent(QGraphicsSceneMouseEvent *e) {
  d->dragmoved = false;
  qDebug() << "Mouse press" << e->pos() << pos();
  d->scene->enablePinHighlighting(false);
  QGraphicsItemGroup::mousePressEvent(e);
}

void SceneElement::mouseMoveEvent(QGraphicsSceneMouseEvent *e) {
  QPointF newpos = pos();
  QPointF oldpos = d->scene->library()->scale()
    * d->scene->circuit().elements()[d->id].position();
  qDebug() << "Mouse move" << newpos << oldpos;
  QGraphicsItemGroup::mouseMoveEvent(e);
  
  if (d->dragmoved || newpos != oldpos) {
    qDebug() << "Position changed";
    d->scene->tentativelyMoveSelection(newpos - oldpos);
    d->dragmoved = true;
  }
}

void SceneElement::mouseReleaseEvent(QGraphicsSceneMouseEvent *e) {
  d->scene->enablePinHighlighting(true);
  auto const &lib = d->scene->library();
  auto const &circ = d->scene->circuit();
  QPointF newpos = pos();
  QPointF oldpos = lib->scale() * circ.elements()[d->id].position();
  qDebug() << "Mouse release" << newpos << oldpos;
  QGraphicsItemGroup::mouseReleaseEvent(e);
  if (d->dragmoved || newpos != oldpos) {
    qDebug() << "Position changed";
    d->scene->moveSelection(newpos - oldpos);
  }
}

void SceneElement::rebuild() {
  auto const &lib = d->scene->library();
  auto const &circ = d->scene->circuit();
  setPos(lib->scale() * circ.elements()[d->id].position());
}

Scene *SceneElement::scene() {
  return d->scene;
}

int SceneElement::id() const {
  return d->id;
}

void SceneElement::showHover() {
  d->showhover = true;
  d->markHover();
}

void SceneElement::hideHover() {
  d->showhover = false;
  d->markHover();
}
 
void SceneElement::hoverEnterEvent(QGraphicsSceneHoverEvent *e) {
  d->hovering = true;
  d->markHover();
  QGraphicsItemGroup::hoverEnterEvent(e);
}

void SceneElement::hoverLeaveEvent(QGraphicsSceneHoverEvent *e) {
  d->hovering =false;
  d->markHover();
  QGraphicsItemGroup::hoverLeaveEvent(e);
}

