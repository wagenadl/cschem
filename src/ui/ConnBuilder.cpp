// ConnBuilder.cpp

#include "ConnBuilder.h"
#include "file/Circuit.h"
#include <QSet>
#include "Scene.h"
#include <QDebug>
#include <QGraphicsSceneMouseEvent>
#include <QKeyEvent>

static QPen defaultPen() {
  QPen p(QColor(0, 0, 0));
  p.setWidthF(1.5);
  return p;
}

class ConnBuilderData {
public:
  ConnBuilderData(Scene *scene): scene(scene), circ(scene->circuit()) {
    fromId = -1;
    toId = -1;
  }
  void buildConnection();
  void ensureStartJunction();
  void ensureEndJunction();
  int ensureJunctionFor(int id, QString pin, QPointF pt);
  QPolygonF simplifiedPoints() const;
  void fixPenultimate();
  QGraphicsLineItem *newSegment();
  bool considerCompletion();
  void forceCompletion(int elt, QString pin);
  void forceDanglingCompletion();
public:
  Scene *scene;
  Circuit circ;
  QSet<int> junctions;
  QSet<int> connections;
  QPolygonF points; // includes from and to
  int fromId, toId; // could refer to a new junction!
  QString fromPin, toPin;
  QList<QGraphicsLineItem *> segments;
};

void ConnBuilderData::fixPenultimate() {
  QLineF l = segments.last()->line();
  points << l.p1();
}

QGraphicsLineItem *ConnBuilderData::newSegment() {
  QLineF l = segments.last()->line();
  auto *gli = new QGraphicsLineItem;
  gli->setPen(defaultPen());
  gli->setLine(QLineF(l.p2(), l.p2()));
  segments << gli;
  return gli;  
}

bool ConnBuilderData::considerCompletion() {
  QLineF l = segments.last()->line();
  int elt = scene->elementAt(l.p2());
  qDebug() << "at" << elt;
  if (elt>0) {
    QString pin = scene->pinAt(l.p2(), elt);
    qDebug() << "at" << elt << pin;
    if (pin != "-") {
      forceCompletion(elt, pin);
      return true;
    }
  }
  return false;
}

void ConnBuilderData::forceCompletion(int elt, QString pin) {
  points << scene->pinPosition(elt, pin);
  toId = elt;
  toPin = pin;
  ensureEndJunction();
  buildConnection();
}

void ConnBuilderData::forceDanglingCompletion() {
  QLineF l = segments.last()->line();
  points << l.p2();
  toId = 0;
  toPin = "";
  buildConnection();
}

QPolygonF ConnBuilderData::simplifiedPoints() const {
  return scene->library()->simplifyPath(points);
}

void ConnBuilderData::buildConnection() {
  if (fromId>0 && toId==fromId && fromPin==toPin) {
    // circular -> abandon
    fromId = -1;
    return;
  }
  Connection c;
  c.setFromId(fromId);
  c.setToId(toId);
  c.setFromPin(fromPin);
  c.setToPin(toPin);
  auto pp = simplifiedPoints();
  qDebug() << fromId << toId << pp;
  if (fromId>0)
    pp.removeFirst();
  if (toId>0)
    pp.removeLast();
  QPolygon via;
  for (auto p: pp)
    via << scene->library()->downscale(p);
  c.setVia(via);
  circ.connections()[c.id()] = c;
  connections << c.id();
}

void ConnBuilderData::ensureStartJunction() {
  if (fromId<=0 || circ.element(fromId).type() == Element::Type::Junction)
    return;

  int jid = ensureJunctionFor(fromId, fromPin, points.first());
  if (jid>0) {
    fromId = jid;
    fromPin = "";
  }
}

void ConnBuilderData::ensureEndJunction() {
  if (toId<=0 || circ.element(toId).type() == Element::Type::Junction)
    return;

  int jid = ensureJunctionFor(toId, toPin, points.last());
  if (jid>0) {
    toId = jid;
    toPin = "";
  }
}

int ConnBuilderData::ensureJunctionFor(int id, QString pin, QPointF pt) {
  QSet<int> othercons = circ.connectionsOn(id, pin);
  if (othercons.isEmpty())
    return -1;

  qDebug() << "othercons" << othercons;
  
  // must add a junction, or this will be confusing
  Element j(Element::junction(scene->library()->downscale(pt)));
  circ.elements()[j.id()] = j;
  junctions << j.id();
  
  // create connection b/w original pin and new junction
  Connection c;
  c.setFromId(id);
  c.setFromPin(pin);
  c.setToId(j.id());
  circ.connections()[c.id()] = c;
  connections << c.id();
  
  // move other connections to junction
  for (int o: othercons) {
    Connection &c(circ.connection(o));
    if (c.fromId() == id) {
      c.setFromId(j.id());
      c.setFromPin("");
    }
    if (c.toId() == id) {
      qDebug() << "rerouting" << c.id() << " from " << c.toId() << "to" << j.id();      c.setToId(j.id());
      c.setToPin("");
    }
    connections << c.id();
  }
  return j.id();
}

ConnBuilder::ConnBuilder(Scene *scene): d(new ConnBuilderData(scene)) {
}

ConnBuilder::~ConnBuilder() {
  delete d;
}

void ConnBuilder::start(QPointF fromPos, int fromId, QString fromPin) {
  for (auto seg: d->segments)
    delete seg;
  d->segments.clear();
  d->junctions.clear();
  d->connections.clear();
  d->fromId = fromId;
  d->fromPin = fromPin;
  d->toId = -1;
  d->toPin = "";

  d->points.clear();
  QPointF p1 = d->scene->library()->nearestGrid(fromPos);
  QPointF p0 = p1;
  if (fromId>0)
    p0 = d->scene->pinPosition(fromId, fromPin);
  d->points << p0;
  
  d->ensureStartJunction();

  auto *gli = new QGraphicsLineItem;
  gli->setPen(defaultPen());
  gli->setLine(QLineF(p0, p1));
  d->segments << gli;
  addToGroup(gli);

  gli = new QGraphicsLineItem;
  gli->setPen(defaultPen());
  gli->setLine(QLineF(p1, p1));
  d->segments << gli;
  addToGroup(gli);
  
  /* Eventually, we should allow fromId to be =0 at this point, and we
     should create a junction to accommodate that. Or perhaps fromId should
     be allowed to be a Connection as well as an Element.
  */
}

void ConnBuilder::keyPress(QKeyEvent *e) {
  qDebug() << "ConnBuilder: key" << e->key();
  switch (e->key()) {
  case Qt::Key_Escape:
    qDebug() << "ConnBuilder: abandon";
    d->fromId = -1; // abandon
    break;
  case Qt::Key_Return:
    qDebug() << "ConnBuilder: complete";
    if (!d->considerCompletion())
      d->forceDanglingCompletion();
    break;
  }
}

void ConnBuilder::mouseMove(QGraphicsSceneMouseEvent *e) {
  qDebug() << "ConnBuilder: move " << e->scenePos();
  QPointF p0 = d->points.last();
  QPointF p = d->scene->library()->nearestGrid(e->scenePos());
  QPointF dp = p - p0;
  int N = d->segments.size();
  auto *gli1 = d->segments[N-2];
  auto *gli2 = d->segments[N-1];
  QPointF p1;
  if (N==2) {
    // arbitrary direction for now
    p1 = (abs(dp.x()) > abs(dp.y()))
      ? QPoint(p.x(), p0.y())
      : QPoint(p0.x(), p.y());
  } else {
    // should we enforce a corner? not right now
    p1 = (abs(dp.x()) > abs(dp.y()))
      ? QPoint(p.x(), p0.y())
      : QPoint(p0.x(), p.y());
  }
  gli1->setLine(QLineF(p0, p1));
  gli2->setLine(QLineF(p1, p));
}

void ConnBuilder::mousePress(QGraphicsSceneMouseEvent *e) {
  qDebug() << "ConnBuilder: press " << e->scenePos();
}

void ConnBuilder::mouseRelease(QGraphicsSceneMouseEvent *e) {
  qDebug() << "ConnBuilder: release " << e->scenePos();
  d->fixPenultimate();
  addToGroup(d->newSegment());
  d->considerCompletion();
}

bool ConnBuilder::isComplete() const {
  return d->toId >= 0 || isAbandoned();
}

bool ConnBuilder::isAbandoned() const {
  return d->fromId < 0;
}

QList<Connection> ConnBuilder::connections() const {
  QList<Connection> lst;
  for (int id: d->connections)
    lst << d->circ.connection(id);
  return lst;
}

QList<Element> ConnBuilder::junctions() const {
  QList<Element> lst;
  for (int id: d->junctions)
    lst << d->circ.element(id);
  return lst;
}
  
