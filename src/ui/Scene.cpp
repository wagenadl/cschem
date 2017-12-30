// Scene.cpp

#include "Scene.h"
#include <QDebug>
#include "SceneElement.h"
#include "SceneConnection.h"
#include "svg/Router.h"
#include <QGraphicsSceneMouseEvent>
#include "HoverPin.h"
#include "ConnBuilder.h"

class SceneData {
public:
  SceneData(PartLibrary const *lib): lib(lib) {
    hoverpin = 0;
    hoverpinEnabled = true;
    connbuilder = 0;
  }
  QPoint pinPosition(int id, QString pin) const {
    if (circ.elements().contains(id))
      return Router(lib).pinPosition(circ.element(id), pin);
    else 
      return QPoint();
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
  void deleteElement(int id);
  void deleteConnection(int id);
  void considerDroppingJunction(int id);
public:
  PartLibrary const *lib;
  Circuit circ;
  QMap<int, class SceneElement *> elts;
  QMap<int, class SceneConnection *> conns;
  QPointF mousexy;
  class HoverPin *hoverpin;
  bool hoverpinEnabled;
  QList<Circuit> undobuffer;
  QList<Circuit> redobuffer;
  ConnBuilder *connbuilder;
};

void SceneData::deleteElement(int id) {
  QSet<int> cc;
  for (auto &c: circ.connections()) {
    if (c.fromId()==id) {
      c.setFromId(0); // make dangling
      c.via().prepend(lib->downscale(pinPosition(id, c.fromPin())));
      cc << c.id();
    }
    if (c.toId()==id) {
      c.setToId(0); // make dangling
      c.via().append(lib->downscale(pinPosition(id, c.toPin())));
      cc << c.id();
    }
  }

  for (int id: cc) 
    conns[id]->rebuild();
  delete elts[id];
  elts.remove(id);
  circ.elements().remove(id);
}  

void SceneData::deleteConnection(int id) {
  auto con(circ.connection(id)); // grab the connection for testing junctions

  delete conns[id];
  conns.remove(id);
  circ.connections().remove(id);
  
  int from = con.fromId();
  if (circ.element(from).type() == Element::Type::Junction)
    considerDroppingJunction(from);

  int to = con.toId();
  if (circ.element(to).type() == Element::Type::Junction) 
    considerDroppingJunction(to);
}

void SceneData::considerDroppingJunction(int id) {
  QList<int> cons = circ.connectionsOn(id, "").toList();
  if (cons.size()==2) {
    // reconnect what would be dangling
    auto con1(circ.connection(cons[0]));
    auto con2(circ.connection(cons[1]));
    if (con1.fromId() == id)
      con1.reverse();
    if (con2.toId() == id)
      con2.reverse();

    con1.setToId(con2.toId());
    con1.setToPin(con2.toPin());
    con1.via() += con2.via();
    circ.connections().remove(cons[1]);
    circ.connection(cons[0]) = con1;
    conns[cons[0]]->rebuild();
    delete conns[cons[1]];
    conns.remove(cons[1]);
  }
  if (cons.size() <= 2)
    deleteElement(id);
}  

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
  return d->pinPosition(partid, pin);
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
  updateOverPin(e->scenePos(), -1, (e->modifiers() & Qt::ShiftModifier)
                || d->connbuilder);
  if (d->connbuilder) {
    qDebug() << "Scene::mousePress";
    d->connbuilder->mousePress(e);
    update();
  } else {
    if (d->hoverpin->element()>0) {
      d->connbuilder = new ConnBuilder(this);
      addItem(d->connbuilder);
      d->connbuilder->start(e->scenePos(),
                            d->hoverpin->element(), d->hoverpin->pinName());
    } else {
      QGraphicsScene::mousePressEvent(e);
    }
  }
}

void Scene::mouseMoveEvent(QGraphicsSceneMouseEvent *e) {
  d->mousexy = e->scenePos();
  updateOverPin(e->scenePos(), -1, e->buttons() != 0
                || (e->modifiers() & Qt::ShiftModifier)
                || d->connbuilder);
  if (d->connbuilder) {
    d->connbuilder->mouseMove(e);
    update();
  } else {
    QGraphicsScene::mouseMoveEvent(e);
  }
}

void Scene::mouseReleaseEvent(QGraphicsSceneMouseEvent *e) {
  d->mousexy = e->scenePos();
  if (d->connbuilder) {
    d->connbuilder->mouseRelease(e);
    update();
    if (d->connbuilder->isComplete())
      finalizeConnection();
  } else {
    QGraphicsScene::mouseReleaseEvent(e);
    updateOverPin(e->scenePos(), -1, e->modifiers() & Qt::ShiftModifier);
  }
}

void Scene::keyPressEvent(QKeyEvent *e) {
  if (d->connbuilder) {
    d->connbuilder->keyPress(e);
    if (d->connbuilder->isComplete())
      finalizeConnection();
    return;
  }
  
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
  case Qt::Key_Delete:
    d->preact();
    d->deleteElement(id);
    break;
  }
}

void Scene::keyPressOnConnection(class SceneConnection *con, QKeyEvent *e) {
  int id = con->id();
  switch (e->key()) {
  case Qt::Key_Delete: {
    d->preact();
    d->deleteConnection(id);
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
    return "-";
  QString sym = d->circ.element(elementId).symbol();
  Part const &part = library()->part(sym);
  double r = library()->scale();
  for (auto p: part.pinNames())
    if (L2(scenepos - pinPosition(elementId, p)) <= 1.1*r*r)
      return p;
  return "-";
}
  

void Scene::finalizeConnection() {
  if (!d->connbuilder->isAbandoned()) {
    d->preact();
    for (auto c: d->connbuilder->junctions()) {
      d->circ.elements()[c.id()] = c;  
      if (d->elts.contains(c.id()))
        delete d->elts[c.id()];
      d->elts[c.id()] = new SceneElement(this, c);
    }
    for (auto c: d->connbuilder->connections()) {
      d->circ.connections()[c.id()] = c;  
      if (d->conns.contains(c.id()))
        delete d->conns[c.id()];
      d->conns[c.id()] = new SceneConnection(this, c);
    }
  }
  delete d->connbuilder;
  d->connbuilder = 0;
}

void Scene::updateOverPin(QPointF p, int elt, bool allowJunction) {
  int elt0 = d->hoverpin->element();
  d->hoverpin->updateHover(p, elt, allowJunction);
  int elt1 = d->hoverpin->element();
  if (elt0!=elt1) {
    if (d->elts.contains(elt0))
      d->elts[elt0]->showHover();
    if (d->elts.contains(elt1) && d->hoverpinEnabled)
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
  d->hoverpinEnabled = hl;
  if (hl)
    d->hoverpin->show();
  else
    d->hoverpin->hide();
}
