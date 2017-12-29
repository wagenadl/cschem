// Scene.cpp

#include "Scene.h"
#include <QDebug>
#include "SceneElement.h"
#include "SceneConnection.h"
#include "svg/Router.h"
#include <QGraphicsSceneMouseEvent>
#include "HoverPin.h"

Scene::Scene(PartLibrary const *lib, QObject *parent):
  QGraphicsScene(parent),
  lib(lib) {
  circ = 0;
  hoverpin = new HoverPin(this);
  addItem(hoverpin);
}

void Scene::setCircuit(Circuit *c) {
  for (auto i: elts)
    delete i;
  elts.clear();
  for (auto i: conns)
    delete i;
  conns.clear();
  circ = c;
  rebuild();
}


void Scene::rebuild() {
  /* We should be able to do better than start afresh in general, but for now: */
  for (auto i: elts)
    delete i;
  elts.clear();
  for (auto i: conns)
    delete i;
  conns.clear();

  if (!circ)
    return;

  for (auto const &c: circ->elements()) 
    elts[c.id()] = new SceneElement(this, c);
  
  for (auto const &c: circ->connections())
    conns[c.id()] = new SceneConnection(this, c);
}

QPoint Scene::pinPosition(int partid, QString pin) const {
  if (circ && circ->elements().contains(partid))
    return Router(lib).pinPosition(circ->element(partid), pin);
  else 
    return QPoint();
}


PartLibrary const *Scene::library() const {
  return lib;
}

Circuit const *Scene::circuit() const {
  return circ;
}

Circuit *Scene::circuit() {
  return circ;
}

QSet<int> Scene::selectedElements() const {
  QSet<int> selection;
  for (int id: elts.keys())
    if (elts[id]->isSelected())
      selection << id;
  return selection;
}

void Scene::tentativelyMoveSelection(QPointF delta) {
  QSet<int> selection = selectedElements();
  QSet<int> internalcons = circ->connectionsIn(selection);
  QSet<int> fromcons = circ->connectionsFrom(selection) - internalcons;
  QSet<int> tocons = circ->connectionsTo(selection) - internalcons;

  for (int id: internalcons)
    conns[id]->temporaryTranslate(delta);
  for (int id: fromcons)
    conns[id]->temporaryTranslateFrom(delta);
  for (int id: tocons)
    conns[id]->temporaryTranslateTo(delta);
}

void Scene::moveSelection(QPointF delta) {
  QPoint dd = (delta/lib->scale()).toPoint();

  QSet<int> selection = selectedElements();
  QSet<int> internalcons = circ->connectionsIn(selection);
  QSet<int> fromcons = circ->connectionsFrom(selection) - internalcons;
  QSet<int> tocons = circ->connectionsTo(selection) - internalcons;
  
  if (!dd.isNull()) {
    // must actually change circuit
    QMap<int, Element> origFrom;
    QMap<int, Element> origTo;
    Circuit origCirc(*circ);

    for (int id: fromcons)
      origFrom[id] = circ->element(circ->connection(id).fromId());
    for (int id: tocons)
      origTo[id] = circ->element(circ->connection(id).toId());
    
    for (int id: selection)
      circ->element(id).translate(dd);
    for (int id: internalcons)
      circ->connection(id).translate(dd);

    Router router(lib);
    for (int id: fromcons) 
      circ->connection(id) = router.reroute(id, origCirc, *circ);
    for (int id: tocons) 
      circ->connection(id) = router.reroute(id, origCirc, *circ);
  }

  for (int id: selection)
    elts[id]->rebuild();

  for (int id: internalcons + fromcons + tocons)
    conns[id]->rebuild();
}

void Scene::mousePressEvent(QGraphicsSceneMouseEvent *e) {
  mousexy = e->scenePos();
  QGraphicsScene::mousePressEvent(e);
}

void Scene::mouseMoveEvent(QGraphicsSceneMouseEvent *e) {
  mousexy = e->scenePos();
  updateOverPin(e->scenePos());
  QGraphicsScene::mouseMoveEvent(e);
}

void Scene::mouseReleaseEvent(QGraphicsSceneMouseEvent *e) {
  mousexy = e->scenePos();
  QGraphicsScene::mouseReleaseEvent(e);
}

void Scene::keyPressEvent(QKeyEvent *e) {
  if (focusItem()==0) {
    QGraphicsItem *item = itemAt(mousexy, QTransform());
    while (item && item->parentItem())
      item = item->parentItem();

    SceneElement *elt = dynamic_cast<SceneElement *>(item);
    SceneConnection *con = dynamic_cast<SceneConnection *>(item);

    if (elt)
      keyPressOnElement(elt, e);
    else if (con)
      keyPressOnConnection(con, e);

    keyPressAnywhere(e);
  } else {
    QGraphicsScene::keyPressEvent(e);
  }
}

void Scene::keyPressOnElement(class SceneElement *elt, QKeyEvent *e) {
  int id = elt->id();
  switch (e->key()) {
  case Qt::Key_Delete: {
    delete elts[id];
    elts.remove(id);
    circ->elements().remove(id);
    QSet<int> cc;
    for (auto &c: circ->connections()) 
      if (c.fromId()==id || c.toId()==id)
        cc << c.id();
    for (int id: cc) {
      delete conns[id];
      conns.remove(id);
      circ->connections().remove(id);
    }
  } break;
  }
}

void Scene::keyPressOnConnection(class SceneConnection *con, QKeyEvent *e) {
  int id = con->id();
  switch (e->key()) {
  case Qt::Key_Delete: {
    delete conns[id];
    conns.remove(id);
    circ->connections().remove(id);
  } break;
  }
}

void Scene::keyPressAnywhere(QKeyEvent *) {
}
  
void Scene::addConnection(int fromPart, QString fromPin, QPointF to) {
  int toPart = -1;
  for (auto e: elts) {
    if (e->boundingRect().contains(e->mapFromScene(to))) {
      toPart = e->id();
      break;
    }
  }
  if (toPart < 0) {
    qDebug() << "Not adding connection";
    return;
  }

  QString sym = circ->element(toPart).symbol();
  Part const &part = library()->part(sym);
  double dist = 1e9;
  QString toPin = "";
  for (auto p: part.pinNames()) {
    double dd = (to - pinPosition(toPart, p)).manhattanLength();
    if (dd<dist) {
      toPin = p;
      dist = dd;
    }
  }
  qDebug() << "Drop near" << toPart << toPin;
  if (toPart != fromPart || toPin != fromPin) {
    // create new connection
    Connection c;
    c.setFromId(fromPart);
    c.setFromPin(fromPin);
    c.setToId(toPart);
    c.setToPin(toPin);
    circ->connections()[c.id()] = c;
    conns[c.id()] = new SceneConnection(this, c);    
  }
}

void Scene::updateOverPin(QPointF p, int elt) {
  hoverpin->updateHover(p, elt);
}

QMap<int, class SceneElement *> const &Scene::elements() const {
  return elts;
}

QMap<int, class SceneConnection *> const &Scene::connections() const {
  return conns;
}
