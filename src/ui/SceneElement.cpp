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
  }
public:
  Scene *scene;
  int id;
  QGraphicsSvgItem *element;
  QMap<QString, QGraphicsEllipseItem *> pins;
  QGraphicsTextItem *name;
  QGraphicsTextItem *value;
  QGraphicsTextItem *label;
public:
  bool dragmoved;
};

SceneElement::SceneElement(class Scene *parent, Element const &elt) {
  d = new SceneElementData;
  //
  d->scene = parent;
  d->id = elt.id();
  QPoint pos = elt.position();
  QString sym = elt.symbol();

  PartLibrary const *lib = d->scene->library();
  Part const &p = lib->part(sym);
  double s = lib->scale();
  if (!p.isValid()) {
    qDebug() << "Cannot find svg for symbol" << sym;
  }

  QSvgRenderer *r = lib->renderer(sym);
  if (!r) {
    qDebug() << "Cannot construct renderer for symbol" << sym;
    return;
  }
  QPoint o = p.origin();
  for (QString name: p.pinNames()) {
    QPoint pt = p.pinPosition(name) - o;
    qDebug() << name << pt;
    auto *e = new QGraphicsEllipseItem(QRectF(pt - QPointF(s,s),
                                              2*QSizeF(s, s)));
    e->setBrush(QBrush(QColor(255, 128, 128, 0))); // initially invisible
    e->setPen(QPen(Qt::NoPen));
    addToGroup(e);
    d->pins[name] = e;
  }

  d->element = new QGraphicsSvgItem;
  d->element->setSharedRenderer(r);
  d->element->setPos(-o);
  addToGroup(d->element);

  parent->addItem(this);
  setPos(s * pos);

  setAcceptHoverEvents(true);
  setFlag(ItemIsMovable);
  setFlag(ItemIsSelectable);
  //  setFlag(ItemSendsGeometryChanges);
  setCacheMode(DeviceCoordinateCache);
}

SceneElement::~SceneElement() {
  delete d;
}

void SceneElement::hoverEnterEvent(QGraphicsSceneHoverEvent *e) {
  qDebug() << "Enter" << d->id << isSelected();
  for (QString pid: d->pins.keys()) {
    auto p = d->pins[pid];
    bool got = false;
    for (auto const &c: d->scene->circuit()->connections()) {
      if ((c.fromId()==d->id && c.fromPin()==pid)
          || (c.toId()==d->id && c.toPin()==pid)) {
        got = true;
        break;
      }
    }
    if (got)
      p->setBrush(QBrush(QColor(128, 255, 128, 128)));
    else
      p->setBrush(QBrush(QColor(255, 128, 128, 128)));
  }
  
  auto *ef = new QGraphicsColorizeEffect;
  ef->setColor(QColor(128, 255, 128));
  ef->setStrength(.5);
  d->element->setGraphicsEffect(ef);
  QGraphicsItemGroup::hoverEnterEvent(e);
}

void SceneElement::hoverLeaveEvent(QGraphicsSceneHoverEvent *e) {
  qDebug() << "Leave" << d->id << isSelected();
  for (auto p: d->pins)
    p->setBrush(QBrush(QColor(255, 128, 128, 0)));
  d->element->setGraphicsEffect(0);
  QGraphicsItemGroup::hoverLeaveEvent(e);
}

void SceneElement::mousePressEvent(QGraphicsSceneMouseEvent *e) {
  d->dragmoved = false;
  qDebug() << "Mouse press" << e->pos() << pos();
  QGraphicsItemGroup::mousePressEvent(e);
}

void SceneElement::mouseMoveEvent(QGraphicsSceneMouseEvent *e) {
  QPointF newpos = pos();
  QPointF oldpos = d->scene->library()->scale()
    * d->scene->circuit()->elements()[d->id].position();
  qDebug() << "Mouse move" << newpos << oldpos;
  QGraphicsItemGroup::mouseMoveEvent(e);

  if (d->dragmoved || newpos != oldpos) {
    qDebug() << "Position changed";
    d->scene->tentativelyMoveSelection(newpos - oldpos);
    d->dragmoved = true;
  }
}

void SceneElement::mouseReleaseEvent(QGraphicsSceneMouseEvent *e) {
  QPointF newpos = pos();
  QPointF oldpos = d->scene->library()->scale()
    * d->scene->circuit()->elements()[d->id].position();
  qDebug() << "Mouse release" << newpos << oldpos;
  QGraphicsItemGroup::mouseReleaseEvent(e);
  if (d->dragmoved || newpos != oldpos) {
    qDebug() << "Position changed";
    d->scene->moveSelection(newpos - oldpos);
  }
}

void SceneElement::rebuild() {
  setPos(d->scene->library()->scale()
         * d->scene->circuit()->elements()[d->id].position());
}

