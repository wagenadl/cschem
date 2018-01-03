// ConnBuilder.cpp

#include "ConnBuilder.h"
#include "file/Circuit.h"
#include <QSet>
#include "Scene.h"
#include <QDebug>
#include <QGraphicsSceneMouseEvent>
#include <QKeyEvent>
#include "svg/Geometry.h"

static QPen defaultPen() {
  QPen p(QColor(0, 0, 0));
  p.setWidthF(1.5);
  return p;
}

class ConnBuilderData {
public:
  ConnBuilderData(Scene *scene): scene(scene), circ(scene->circuit()) {
    reset();
  }
  void reset();
  void buildConnection();
  void ensureStartJunction();
  void ensureEndJunction();
  int ensureJunctionFor(int id, QString pin, QPointF pt);
  QPolygonF simplifiedPoints() const;
  bool fixPenultimate(); // false if there is nothing to fix yet
  QGraphicsLineItem *newSegment();
  bool considerCompletion();
  void forceCompletion(int elt, QString pin);
  void forceDanglingCompletion();
public:
  Scene *scene;
  Circuit circ;
  QSet<int> junctions;
  QSet<int> connections;
  int majorcon;
  QPolygonF points; // includes from and to
  int fromId, toId; // could refer to a new junction!
  QString fromPin, toPin;
  QList<QGraphicsLineItem *> segments;
};

void ConnBuilderData::reset() {
  fromId = -1;
  fromPin = -1;
  toId = -1;
  toPin = "";
  majorcon = -1;

  for (auto seg: segments)
    delete seg;

  segments.clear();
  junctions.clear();
  connections.clear();
  points.clear();
}

bool ConnBuilderData::fixPenultimate() {
  if (segments.size()==2
      && segments[0]->line().length() < scene->library()->scale()
      && segments[1]->line().length() < scene->library()->scale())
    return false;
      
  QLineF l = segments.last()->line();
  points << l.p1();
  return true;
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
  int seg;
  int c = scene->connectionAt(l.p2(), &seg);
  if (c>0) {
    Connection tgt(circ.connection(c));
    if (tgt.danglingStart())
      seg++; // so seg=0 is always pre-vias
    Element junc = Element::junction(scene->library()->downscale(l.p2()));
    Connection tgt1;
    tgt1.setTo(tgt.to());
    tgt.setTo(junc.id());
    tgt1.setFrom(junc.id());
    QPolygon via = tgt.via();
    QPolygon via1;
    while (via.size() > seg)
      via1.prepend(via.takeLast());
    if (via.size()>=1 && via.last()==junc.position())
      via.removeLast();
    if (via1.size()>=1 && via1.first()==junc.position())
      via1.removeFirst();
    tgt.setVia(via);
    tgt1.setVia(via1);

    qDebug() << "onto line" << junc.report();
    qDebug() << "  " << tgt.report() << tgt.isNull() << tgt.isCircular(); 
    qDebug() << "  " << tgt1.report() << tgt1.isNull() << tgt1.isCircular(); 

    if (tgt.isCircular())
      circ.remove(tgt.id());
    else
      circ.insert(tgt);
    connections << tgt.id();
    if (!tgt1.isCircular()) {
      circ.insert(tgt1);
      connections << tgt1.id();
    }
    circ.insert(junc);
    junctions << junc.id();
    forceCompletion(junc.id(), "");
    return true;
  }
  return false;
}

void ConnBuilderData::forceCompletion(int elt, QString pin) {
  if (elt>0)
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
  circ.insert(c);
  connections << c.id();
  majorcon = c.id();
}

void ConnBuilderData::ensureStartJunction() {
  int jid = ensureJunctionFor(fromId, fromPin, points.first());
  if (jid>0) {
    fromId = jid;
    fromPin = "";
  }
}

void ConnBuilderData::ensureEndJunction() {
  int jid = ensureJunctionFor(toId, toPin, points.last());
  if (jid>0) {
    toId = jid;
    toPin = "";
  }
}

int ConnBuilderData::ensureJunctionFor(int id, QString pin, QPointF pt) {
  // Creates a junction at a pin of an existing element to avoid
  // multiple connections to that pin.
  if (id<=0)
    return -1; // don't worry if dangling
  if (circ.element(id).type() == Element::Type::Junction)
    return -1; // easy if already junction
  QSet<int> othercons = circ.connectionsOn(id, pin);
  if (othercons.isEmpty())
    return -1; // nothing there, so no need

  qDebug() << "othercons" << othercons;
  
  // must add a junction, or this will be confusing
  Element j(Element::junction(scene->library()->downscale(pt)));
  circ.insert(j);
  junctions << j.id();
  
  // create connection b/w original pin and new junction
  Connection c;
  c.setFromId(id);
  c.setFromPin(pin);
  c.setToId(j.id());
  circ.insert(c);
  connections << c.id();
  
  // move other connections to junction
  for (int o: othercons) {
    Connection c(circ.connection(o));
    if (c.fromId() == id) {
      c.setFromId(j.id());
      c.setFromPin("");
    }
    if (c.toId() == id) {
      qDebug() << "rerouting" << c.id() << " from " << c.toId() << "to" << j.id();      c.setToId(j.id());
      c.setToPin("");
    }
    circ.insert(c);
    connections << c.id();
  }
  return j.id();
}

ConnBuilder::ConnBuilder(Scene *scene): d(new ConnBuilderData(scene)) {
}

ConnBuilder::~ConnBuilder() {
  delete d;
}

void ConnBuilder::startFromConnection(QPointF fromPos, int conId, int seg) {
  Geometry geom(d->circ, d->scene->library());
  QPolygon path(geom.connectionPath(conId));
  QPoint from = d->scene->library()->downscale(fromPos);
  qDebug() << "startFromConnection" << from << seg << path;
  QPoint p0 = path[seg];
  QPoint p1 = path[seg+1];
  QPoint dir = p1 - p0;
  dir /= dir.manhattanLength();
  int along = QPoint::dotProduct(from - p0, dir);
  from = p0 + along * dir;
  qDebug() << "dir" << dir << " newfrom" << from;
  // now "from" is on the line
  qDebug() << QPoint::dotProduct(from - p0, dir);
  qDebug() << QPoint::dotProduct(from - p1, dir);
  Element junc(Element::junction(from));
  Connection con(d->circ.connection(conId));

  // for now:
  d->circ.insert(junc);
  startFromPin(fromPos, junc.id(), "");
  d->junctions << junc.id();
}

void ConnBuilder::startFromPin(QPointF fromPos, int fromId, QString fromPin) {
  d->reset();

  d->fromId = fromId;
  d->fromPin = fromPin;

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
    d->fixPenultimate();
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
  int L = d->points.size();
  qDebug() << N << L;
  if (N==2 || L<2) {
    // arbitrary direction for now
    p1 = (abs(dp.x()) > abs(dp.y()))
      ? QPoint(p.x(), p0.y())
      : QPoint(p0.x(), p.y());
  } else {
    QPointF dp0 = p0 - d->points[L-2];
    // enforce a corner
    p1 = (abs(dp0.x()) < abs(dp0.y()))
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
  if (!d->fixPenultimate())
    return;
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
  if (d->majorcon > 0)
    lst << d->circ.connection(d->majorcon);
  for (int id: d->connections)
    if (id != d->majorcon)
      lst << d->circ.connection(id);
  return lst;
}

QList<Element> ConnBuilder::junctions() const {
  QList<Element> lst;
  for (int id: d->junctions)
    lst << d->circ.element(id);
  return lst;
}
  
