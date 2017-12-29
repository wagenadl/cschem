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

class SceneElementPin: public QGraphicsEllipseItem {
public:
  SceneElementPin(QPointF c, double r, QString id, SceneElement *parent):
    QGraphicsEllipseItem(QRectF(c - QPointF(r, r), 2*QSizeF(r, r))),
    id(id), parent(parent) {
    setBrush(QBrush(QColor(255, 128, 128, 0))); // initially invisible
    setPen(QPen(Qt::NoPen));
  }  
private:
  QString id;
  SceneElement *parent;
};



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
    onpin = false;
    dragline = 0;
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
  bool onpin;
  QString onpinId;
  bool dragmoved;
  QGraphicsLineItem *dragline;
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
    auto *e = new SceneElementPin(pt, s, name, this);
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
  setCacheMode(DeviceCoordinateCache);
}

SceneElement::~SceneElement() {
  delete d;
}

void SceneElement::showPin(QString pid) {
  qDebug() << "show pin" << d->id << isSelected();
  if (!d->pins.contains(pid))
    return;
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

void SceneElement::hidePin(QString pid) {
  qDebug() << "Hide pin" << d->id << isSelected();
  if (!d->pins.contains(pid))
    return;
  auto p = d->pins[pid];
  p->setBrush(QBrush(QColor(255, 128, 128, 0)));
}

static double L2(QPointF p) {
  return p.x()*p.x() + p.y()*p.y();
}

void SceneElement::mousePressEvent(QGraphicsSceneMouseEvent *e) {
  d->dragmoved = false;
  d->onpin = false;
  qDebug() << "Mouse press" << e->pos() << pos();
  QPointF pinpos;
  if (scene()->circuit()->element(d->id).type() != Element::Type::Junction) {
    double s = scene()->library()->scale();
    for (QString pid: d->pins.keys()) {
      pinpos = mapFromScene(scene()->pinPosition(d->id, pid));
      qDebug() << pid << "?" << pinpos << e->pos() << s;
      if (L2(pinpos - e->pos()) <= s*s) {
        d->onpin = true;
        d->onpinId = pid;
        qDebug() << "  on pin" << pid;
        break;
      }
    }
  }
  
  if (d->onpin) {
    if (d->dragline)
      qDebug() << "Previous dragline exists";
    d->dragline = new QGraphicsLineItem(QLineF(pinpos, e->pos()), this);
    qDebug() << d->dragline->line();
    addToGroup(d->dragline);
  } else {
    QGraphicsItemGroup::mousePressEvent(e);
  }
}

void SceneElement::mouseMoveEvent(QGraphicsSceneMouseEvent *e) {
  if (d->onpin) {
    qDebug() << "Mouse move on pin";
    if (d->dragline) {
      QLineF l = d->dragline->line();
      d->dragline->setLine(QLineF(l.p1(), e->pos()));
    } else {
      qDebug() << "no drag line??";
    }
  } else {
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
}

void SceneElement::mouseReleaseEvent(QGraphicsSceneMouseEvent *e) {
  if (d->onpin) {
    qDebug() << "Mouse release on pin";
    if (d->dragline) {
      delete d->dragline;
      d->dragline = 0;
      scene()->addConnection(d->id, d->onpinId, e->scenePos());
    } else {
      qDebug() << "No drag line??";
    }
  } else {
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
}

void SceneElement::rebuild() {
  setPos(d->scene->library()->scale()
         * d->scene->circuit()->elements()[d->id].position());
}

Scene *SceneElement::scene() {
  return d->scene;
}

int SceneElement::id() const {
  return d->id;
}

