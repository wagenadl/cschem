// Scene.cpp

#include "Scene.h"
#include <QDebug>
#include "SceneElement.h"
#include "SceneConnection.h"

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
  if (!circ)
    return QPoint();
  Part part;
  QPoint pos;
  if (circ->elements().contains(partid)) {
    Element const &c(circ->elements()[partid]);
    Part const &part = lib->part(c.symbol());
    QPoint pos = c.position();
    return lib->scale() * pos + part.pinPosition(pin) - part.origin();
  } else {
    return QPoint();
  }
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

void Scene::moveSelection(QPointF delta) {
  QPoint dd = (delta/lib->scale()).toPoint();

  QSet<int> selection;
  for (int id: elts.keys())
    if (elts[id]->isSelected())
      selection << id;
    
  if (!dd.isNull()) {
    // must actually change circuit
    for (int id: selection)
      circ->elements()[id].translate(dd);
    for (int id: circ->connectionsIn(selection))
      circ->connections()[id].translate(dd);
  }

  for (int id: selection)
    elts[id]->rebuild();

  QSet<int> cids;
  cids = circ->connectionsFrom(selection);
  cids += circ->connectionsTo(selection);
  for (int id: cids)
    conns[id]->rebuild();
}
