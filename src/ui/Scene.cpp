// Scene.cpp

#include "Scene.h"
#include <QDebug>
#include <QGraphicsSvgItem>
#include <QGraphicsEllipseItem>
#include <QGraphicsItemGroup>
#include <QGraphicsPathItem>

Scene::Scene(PartLibrary const &lib, QObject *parent):
  QGraphicsScene(parent),
  lib(lib) {
  circuit = 0;
}

void Scene::setCircuit(Circuit *c) {
  for (auto i: items)
    delete i;
  items.clear();
  circuit = c;
  rebuild();
}

void Scene::createSvgItem(int id, QPoint pos, QString typ) {
  double s = lib.scale();
  Part const &p = lib.part(typ);
  if (p.isValid()) {
    QSvgRenderer *r = lib.renderer(typ);
    if (!r)
      qDebug() << "Cannot construct renderer for component. Will crash";
    auto *group = new QGraphicsItemGroup;
    QPoint o = p.origin();
    QList<QAbstractGraphicsShapeItem *> pins;
    for (QString name: p.pinNames()) {
      QPoint pt = p.pinPosition(name) - o;
      qDebug() << name << pt;
      pins << new QGraphicsEllipseItem(QRectF(pt - QPointF(s,s),
                                              2*QSizeF(s, s)),
                                       group);
    }
    for (auto p: pins)
      p->setBrush(QBrush(QColor(255, 128, 128, 128)));
    for (auto p: pins)
      p->setPen(QPen(Qt::NoPen));

    auto *item = new QGraphicsSvgItem(group);
    item->setSharedRenderer(r);
    item->setPos(-o);

    addItem(group);
    group->setPos(s * pos);
    items[id] = group;
  } else {
    qDebug() << "Cannot render" << typ;
  }
}
  

void Scene::rebuild() {
  /* We should be able to do better than start afresh in general, but for now: */
  for (auto i: items)
    delete i;
  items.clear();

  if (!circuit)
    return;

  for (auto &c: circuit->components())
    createSvgItem(c.id(), c.position(), "part:" + c.type());
  
  for (auto &c: circuit->ports()) 
    createSvgItem(c.id(), c.position(), "port:" + c.type());

  for (auto &c: circuit->junctions()) 
    createSvgItem(c.id(), c.position(), "junction");

  for (auto &c: circuit->connections())
    createConnection(c);
}

QPoint Scene::pinPosition(int partid, QString pin) const {
  if (!circuit)
    return QPoint();
  Part part;
  QPoint pos;
  if (circuit->components().contains(partid)) {
    Component const &c(circuit->components()[partid]);
    part = lib.part("part:" + c.type());
    pos = c.position();
  } else if (circuit->ports().contains(partid)) {
    Port const &c(circuit->ports()[partid]);
    part = lib.part("port:" + c.type());
    pos = c.position();
  } else if (circuit->junctions().contains(partid)) {
    Junction const &c(circuit->junctions()[partid]);
    part = lib.part("junction");
    pos = c.position();
  }
  if (part.isValid())
    return lib.scale() * pos + part.pinPosition(pin) - part.origin();
  else
    return QPoint();
}

void Scene::createConnection(Connection const &c) {
  QPointF x0 = pinPosition(c.fromId(), c.fromPin());
  QPointF x1 = pinPosition(c.toId(), c.toPin());
  qDebug() << c.fromId() << c.fromPin() << x0 << c.toId() << c.toPin() << x1;
  qDebug() << c.via();
  QPainterPath pp(x0);
  for (QPoint p: c.via())
    pp.lineTo(lib.scale()*p);
  pp.lineTo(x1);
  
  auto *item = new QGraphicsPathItem(pp);
  item->setPen(QPen(QColor(0,0,0), lib.scale()/2.0,
                     Qt::SolidLine, Qt::SquareCap, Qt::MiterJoin));
  addItem(item);
  items[c.id()] = item;
}
