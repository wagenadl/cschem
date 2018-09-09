// ConnBuilder.cpp

#include "ConnBuilder.h"
#include "circuit/Circuit.h"
#include <QSet>
#include "Scene.h"
#include <QDebug>
#include <QGraphicsSceneMouseEvent>
#include <QKeyEvent>
#include "svg/Geometry.h"
#include "circuit/CircuitMod.h"

static QPen defaultPen() {
  QPen p(QColor(0, 0, 0));
  p.setWidthF(1.5);
  return p;
}

class ConnBuilderData {
public:
  ConnBuilderData(Scene *scene):
    scene(scene),
    circ(scene->circuit()),
    lib(scene->library()) {
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
  SymbolLibrary const &lib;
  QSet<int> junctions;
  QSet<int> connections;
  QList<int> dropped;
  int majorcon;
  QPolygonF points; // includes from and to
  int fromId, toId; // could refer to a new junction!
  QString fromPin, toPin;
  QList<QGraphicsLineItem *> segments;
  bool horstart, verstart;
};

void ConnBuilderData::reset() {
  fromId = -1;
  fromPin = -1;
  toId = -1;
  toPin = "";
  majorcon = -1;
  horstart = verstart = false;

  for (auto seg: segments)
    delete seg;

  segments.clear();
  junctions.clear();
  connections.clear();
  points.clear();
}

bool ConnBuilderData::fixPenultimate() {
  if (segments.size()==2
      && segments[0]->line().length() < scene->library().scale()
      && segments[1]->line().length() < scene->library().scale())
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
  if (elt>0) {
    QString pin = scene->pinAt(l.p2(), elt);
    if (pin != PinID::NOPIN) {
      forceCompletion(elt, pin);
      return true;
    }
  }
  int seg;
  int c = scene->connectionAt(l.p2(), &seg);
  if (c>0) {
    CircuitMod cm(circ, lib);
    int junc = cm.injectJunction(c, lib.downscale(l.p2()));
    if (junc>0) {
      circ = cm.circuit();
      junctions += cm.affectedElements();
      connections += cm.affectedConnections();
      forceCompletion(junc, "");
      return true;
    }
  }
  return false;
}

void ConnBuilderData::forceCompletion(int elt, QString pin) {
  if (elt>0)
    points << scene->pinPosition(elt, pin);
  toId = elt;
  toPin = pin;

  if (fromId>0 && toId==fromId && fromPin==toPin) {
    // circular -> abandon
    fromId = -1;
    return;
  }

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
  return scene->library().simplifyPath(points);
}

void ConnBuilderData::buildConnection() {
  if (fromId>0 && toId==fromId && fromPin==toPin) {
    // circular -> abandon
    fromId = -1;
    return;
  }

  if (toId>0 && fromId>0) {
    /* If the exact same connection already exists, drop that connection
       instead of inserting a new one. */
    QSet<int> otherConn = circ.connectionsOn(toId, toPin);
    for (int id: otherConn) {
      Connection const &con(circ.connections[id]);
      if ((con.fromId==fromId && con.fromPin==fromPin
           && con.toId==toId && con.toPin==toPin)
          || (con.toId==fromId && con.toPin==fromPin
           && con.fromId==toId && con.fromPin==toPin)) {
        // gotcha
        dropped << id;
        return;
      }
    }
  }
  
  
  Connection c;
  c.fromId = fromId;
  c.toId = toId;
  c.fromPin = fromPin;
  c.toPin = toPin;
  auto pp = simplifiedPoints();
  if (fromId>0)
    pp.removeFirst();
  if (toId>0)
    pp.removeLast();
  c.via.clear();
  for (auto p: pp)
    c.via << scene->library().downscale(p);
  circ.insert(c);
  connections << c.id;
  majorcon = c.id;
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
  if (circ.elements[id].type == Element::Type::Junction)
    return -1; // easy if already junction
  QSet<int> othercons = circ.connectionsOn(id, pin);
  if (othercons.isEmpty())
    return -1; // nothing there, so no need

  // must add a junction, or this will be confusing
  Element j(Element::junction(scene->library().downscale(pt)));
  circ.insert(j);
  junctions << j.id;
  
  // create connection b/w original pin and new junction
  Connection c;
  c.setFrom(id, pin);
  c.setTo(j.id);
  circ.insert(c);
  connections << c.id;
  
  // move other connections to junction
  for (int o: othercons) {
    Connection c(circ.connections[o]);
    if (c.fromId==id && c.fromPin==pin)
      c.setFrom(j.id);
    if (c.toId==id && c.toPin==pin) 
      c.setTo(j.id);
    circ.insert(c);
    connections << c.id;
  }

  return j.id;
}

ConnBuilder::ConnBuilder(Scene *scene): d(new ConnBuilderData(scene)) {
}

ConnBuilder::~ConnBuilder() {
  delete d;
}

void ConnBuilder::startFromConnection(QPointF fromPos, int conId, int seg) {
  CircuitMod cm(d->circ, d->lib);
  int junc = cm.injectJunction(conId, d->lib.downscale(fromPos));
  if (junc>0) {
    d->circ = cm.circuit();
    startFromPin(fromPos, junc, "");
    d->junctions += cm.affectedElements();
    d->connections += cm.affectedConnections();
  } else {
    qDebug() << "Failed to inject junction";
  }
}

void ConnBuilder::startFromPin(QPointF fromPos, int fromId, QString fromPin) {
  d->reset();

  d->fromId = fromId;
  d->fromPin = fromPin;

  QPointF p1 = d->lib.nearestGrid(fromPos);
  QPointF p0 = p1;
  if (fromId>0) {
    Geometry geom(d->circ, d->lib);
    p0 = d->lib.upscale(geom.pinPosition(fromId, fromPin));
  }
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
  switch (e->key()) {
  case Qt::Key_Escape:
    d->fromId = -1; // abandon
    break;
  case Qt::Key_Return:
    d->fixPenultimate();
    if (!d->considerCompletion())
      d->forceDanglingCompletion();
    break;
  }
}

void ConnBuilder::mouseMove(QGraphicsSceneMouseEvent *e) {
  if (d->points.isEmpty())
    return;
  QPointF p0 = d->points.last();
  QPointF p = d->lib.nearestGrid(e->scenePos());
  QPointF dp = p - p0;
  int N = d->segments.size();
  auto *gli1 = d->segments[N-2];
  auto *gli2 = d->segments[N-1];
  QPointF p1;
  int L = d->points.size();
  if (N==2 || L<2) {
    if (!d->verstart && !d->horstart) {
      if (dp.manhattanLength() > 15) {
        if (abs(dp.x()) > abs(dp.y()))
          d->horstart = true;
        else
          d->verstart = true;
      }
    }
    bool usehor = (d->horstart || abs(dp.x()) > abs(dp.y())) && !d->verstart;
    p1 = usehor ? QPoint(p.x(), p0.y()) : QPoint(p0.x(), p.y());
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
}

void ConnBuilder::mouseRelease(QGraphicsSceneMouseEvent *e) {
  if (d->points.isEmpty())
    return;
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

QList<int> ConnBuilder::droppedConnections() const {
  return d->dropped;
}

QList<Connection> ConnBuilder::connections() const {
  QList<Connection> lst;
  if (d->majorcon > 0)
    lst << d->circ.connections[d->majorcon];
  for (int id: d->connections)
    if (id != d->majorcon)
      lst << d->circ.connections[id];
  return lst;
}

QList<Element> ConnBuilder::junctions() const {
  QList<Element> lst;
  for (int id: d->junctions)
    lst << d->circ.elements[id];
  return lst;
}
  
