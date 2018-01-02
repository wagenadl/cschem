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
    hover = false;
  }
public:
  void markHover() {
    if (hover) {
      auto *ef = new QGraphicsColorizeEffect;
      ef->setColor(QColor(64, 192, 255));
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
  bool hover;
};

SceneElement::SceneElement(class Scene *parent, Element const &elt):
  d(new SceneElementData) {
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

  QSvgRenderer *r = lib->renderer(sym);

  d->element = new QGraphicsSvgItem;
  if (r)
    d->element->setSharedRenderer(r);
  else
    qDebug() << "Cannot construct renderer for symbol" << sym;    
  d->element->setPos(part.shiftedBBox().topLeft());
  addToGroup(d->element);

  parent->addItem(this);
  setPos(s * pos);

  if (sym == "junction")
    setZValue(20);
  else
    setZValue(10);
  
  setFlag(ItemIsMovable);
  setFlag(ItemIsSelectable);
  setCacheMode(DeviceCoordinateCache);
}

SceneElement::~SceneElement() {
  // delete d;
}

// static double L2(QPointF p) {
//   return p.x()*p.x() + p.y()*p.y();
// }

void SceneElement::mousePressEvent(QGraphicsSceneMouseEvent *e) {
  d->dragmoved = false;
  qDebug() << "Mouse press" << e->pos() << pos();
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


void SceneElement::hover() {
  if (!d->hover) {
    d->hover = true;
    d->markHover();
  }
}

void SceneElement::unhover() {
  if (d->hover) {
    d->hover = false;
    d->markHover();
  }
}

SceneElement::WeakPtr::WeakPtr(SceneElement *s,
				  QSharedPointer<SceneElementData> const &d):
  s(s), d(d) {
}

SceneElement::WeakPtr::WeakPtr(): s(0) {
}

SceneElement *SceneElement::WeakPtr::data() const {
  return d.isNull() ? 0 : s;
}

void SceneElement::WeakPtr::clear() {
  d.clear();
}

SceneElement::WeakPtr::operator bool() const {
  return !d.isNull();
}


SceneElement::WeakPtr SceneElement::weakref() {
  return SceneElement::WeakPtr(this, d);
}
