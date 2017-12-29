// Scene.cpp

#include "Scene.h"
#include <QDebug>
#include "SceneElement.h"
#include "SceneConnection.h"
#include "svg/Router.h"

Scene::Scene(PartLibrary const *lib, QObject *parent):
  QGraphicsScene(parent),
  lib(lib) {
  circ = 0;
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

void Scene::keyPressEvent(QKeyEvent *e) {
  qDebug() << "Scene::keypress" << e->key() ;
}

