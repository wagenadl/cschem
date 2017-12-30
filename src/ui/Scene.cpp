// Scene.cpp

#include "Scene.h"
#include <QDebug>
#include "SceneElement.h"
#include "SceneConnection.h"
#include "svg/Router.h"
#include <QGraphicsSceneMouseEvent>
#include "HoverPin.h"

class SceneData {
public:
  SceneData(PartLibrary const *lib): lib(lib) {
    hoverpin = 0;
    hoverpinenabled = true;
  }
  bool undo() {
    if (undobuffer.isEmpty())
      return false;
    redobuffer << circ;
    circ = undobuffer.takeLast();
    return true;
  }
  bool redo() {
    if (redobuffer.isEmpty())
      return false;
    undobuffer << circ;
    circ = redobuffer.takeLast();
    return true;
  }
  void preact() {
    undobuffer << circ;
    redobuffer.clear();
  }
public:
  PartLibrary const *lib;
  Circuit circ;
  QMap<int, class SceneElement *> elts;
  QMap<int, class SceneConnection *> conns;
  QPointF mousexy;
  class HoverPin *hoverpin;
  bool hoverpinenabled;
  QList<Circuit> undobuffer;
  QList<Circuit> redobuffer;
};

Scene::~Scene() {
  delete d;
}
  
Scene::Scene(PartLibrary const *lib, QObject *parent):
  QGraphicsScene(parent) {
  d = new SceneData(lib);
  d->hoverpin = new HoverPin(this);
  addItem(d->hoverpin);
}

void Scene::setCircuit(Circuit const &c) {
  for (auto i: d->elts)
    delete i;
  d->elts.clear();
  for (auto i: d->conns)
    delete i;
  d->conns.clear();
  d->circ = c;
  rebuild();
}


void Scene::rebuild() {
  /* We should be able to do better than start afresh in general, but for now: */
  for (auto i: d->elts)
    delete i;
  d->elts.clear();
  for (auto i: d->conns)
    delete i;
  d->conns.clear();

  for (auto const &c: d->circ.elements()) 
    d->elts[c.id()] = new SceneElement(this, c);
  
  for (auto const &c: d->circ.connections())
    d->conns[c.id()] = new SceneConnection(this, c);
}

QPoint Scene::pinPosition(int partid, QString pin) const {
  if (d->circ.elements().contains(partid))
    return Router(d->lib).pinPosition(d->circ.element(partid), pin);
  else 
    return QPoint();
}


PartLibrary const *Scene::library() const {
  return d->lib;
}

Circuit const &Scene::circuit() const {
  return d->circ;
}

Circuit &Scene::circuit() {
  return d->circ;
}

QSet<int> Scene::selectedElements() const {
  QSet<int> selection;
  for (int id: d->elts.keys())
    if (d->elts[id]->isSelected())
      selection << id;
  return selection;
}

void Scene::tentativelyMoveSelection(QPointF delta) {
  QSet<int> selection = selectedElements();
  QSet<int> internalcons = d->circ.connectionsIn(selection);
  QSet<int> fromcons = d->circ.connectionsFrom(selection) - internalcons;
  QSet<int> tocons = d->circ.connectionsTo(selection) - internalcons;

  for (int id: internalcons)
    d->conns[id]->temporaryTranslate(delta);
  for (int id: fromcons)
    d->conns[id]->temporaryTranslateFrom(delta);
  for (int id: tocons)
    d->conns[id]->temporaryTranslateTo(delta);
}

void Scene::moveSelection(QPointF delta) {
  QPoint dd = (delta/d->lib->scale()).toPoint();

  QSet<int> selection = selectedElements();
  QSet<int> internalcons = d->circ.connectionsIn(selection);
  QSet<int> fromcons = d->circ.connectionsFrom(selection) - internalcons;
  QSet<int> tocons = d->circ.connectionsTo(selection) - internalcons;
  
  if (!dd.isNull()) {
    // must actually change circuit
    d->preact();
    QMap<int, Element> origFrom;
    QMap<int, Element> origTo;
    Circuit origCirc(d->circ);

    for (int id: fromcons)
      origFrom[id] = d->circ.element(d->circ.connection(id).fromId());
    for (int id: tocons)
      origTo[id] = d->circ.element(d->circ.connection(id).toId());
    
    for (int id: selection)
      d->circ.element(id).translate(dd);
    for (int id: internalcons)
      d->circ.connection(id).translate(dd);

    Router router(d->lib);
    for (int id: fromcons) 
      d->circ.connection(id) = router.reroute(id, origCirc, d->circ);
    for (int id: tocons) 
      d->circ.connection(id) = router.reroute(id, origCirc, d->circ);
  }

  for (int id: selection)
    d->elts[id]->rebuild();

  for (int id: internalcons + fromcons + tocons)
    d->conns[id]->rebuild();
}

void Scene::mousePressEvent(QGraphicsSceneMouseEvent *e) {
  d->mousexy = e->scenePos();
  QGraphicsScene::mousePressEvent(e);
}

void Scene::mouseMoveEvent(QGraphicsSceneMouseEvent *e) {
  d->mousexy = e->scenePos();
  updateOverPin(e->scenePos(), -1, e->buttons()!=0);
  QGraphicsScene::mouseMoveEvent(e);
}

void Scene::mouseReleaseEvent(QGraphicsSceneMouseEvent *e) {
  d->mousexy = e->scenePos();
  QGraphicsScene::mouseReleaseEvent(e);
}

void Scene::keyPressEvent(QKeyEvent *e) {
  if (focusItem()==0) {
    QGraphicsItem *item = itemAt(d->mousexy, QTransform());
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
    d->preact();
    delete d->elts[id];
    d->elts.remove(id);
    d->circ.elements().remove(id);
    QSet<int> cc;
    for (auto &c: d->circ.connections()) 
      if (c.fromId()==id || c.toId()==id)
        cc << c.id();
    for (int id: cc) {
      delete d->conns[id];
      d->conns.remove(id);
      d->circ.connections().remove(id);
    }
  } break;
  }
}

void Scene::keyPressOnConnection(class SceneConnection *con, QKeyEvent *e) {
  int id = con->id();
  switch (e->key()) {
  case Qt::Key_Delete: {
    d->preact();
    delete d->conns[id];
    d->conns.remove(id);
    d->circ.connections().remove(id);
  } break;
  }
}

void Scene::keyPressAnywhere(QKeyEvent *e) {
  switch (e->key()) {
  case Qt::Key_Z:
    if (e->modifiers() & Qt::ControlModifier) {
      if (e->modifiers() & Qt::ShiftModifier) {
	if (d->redo())
	  rebuild();
      } else {
	if (d->undo())
	  rebuild();
      }
    }
    break;
  }
}

int Scene::elementAt(QPointF scenepos) const {
  for (auto e: d->elts)
    if (e->boundingRect().contains(e->mapFromScene(scenepos)))
      return e->id();
  return -1;
}

static double L2(QPointF p) {
  return p.x()*p.x() + p.y()*p.y();
}

QString Scene::pinAt(QPointF scenepos, int elementId) const {
  if (!d->elts.contains(elementId))
    return "";
  QString sym = d->circ.element(elementId).symbol();
  Part const &part = library()->part(sym);
  double r = library()->scale();
  for (auto p: part.pinNames()) 
    if (L2(scenepos - pinPosition(elementId, p)) < r*r)
      return p;
  return "-";
}
  
void Scene::addConnection(int fromPart, QString fromPin, QPointF to) {
  int toPart = elementAt(to);
  QString toPin = pinAt(to, toPart);

  if (toPart>0 && toPin!="-") {
    finalizeConnection(fromPart, fromPin, toPart, toPin);
  } else {
    // keep going
  }
}

void Scene::finalizeConnection(int fromPart, QString fromPin,
			       int toPart, QString toPin) {
  if (toPart == fromPart && toPin != fromPin)
    return; // circular

  d->preact();
  
  Connection c = Router(d->lib).autoroute(d->circ.element(fromPart), fromPin,
					  d->circ.element(toPart), toPin,
					  d->circ);
  d->circ.connections()[c.id()] = c;  
  d->conns[c.id()] = new SceneConnection(this, c);    
}

void Scene::updateOverPin(QPointF p, int elt, bool allowJunction) {
  int elt0 = d->hoverpin->element();
  d->hoverpin->updateHover(p, elt, allowJunction);
  int elt1 = d->hoverpin->element();
  if (elt0!=elt1) {
    if (d->elts.contains(elt0))
      d->elts[elt0]->showHover();
    if (d->elts.contains(elt1) && d->hoverpinenabled)
      d->elts[elt1]->hideHover();
  }
}

QMap<int, class SceneElement *> const &Scene::elements() const {
  return d->elts;
}

QMap<int, class SceneConnection *> const &Scene::connections() const {
  return d->conns;
}

void Scene::enablePinHighlighting(bool hl) {
  d->hoverpinenabled = hl;
  if (hl)
    d->hoverpin->show();
  else
    d->hoverpin->hide();
}
