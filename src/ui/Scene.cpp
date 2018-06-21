// Scene.cpp

#include "Scene.h"
#include <QDebug>
#include "SceneElement.h"
#include "SceneConnection.h"
#include <QGraphicsSceneMouseEvent>
#include "HoverManager.h"
#include "ConnBuilder.h"
#include "circuit/CircuitMod.h"
#include "svg/Geometry.h"
#include "Clipboard.h"
#include <QMimeData>
#include "SceneAnnotation.h"
#include "FloatingSymbol.h"
#include "PartList.h"
#include "svg/PackageLibrary.h"

class SceneData {
public:
  SceneData(Scene *scene, Schem const &schem):
    scene(scene), schem(schem) {
    hovermanager = 0;
    connbuilder = 0;
    dragin = 0;
    partlist = 0;
    rubberband = 0;
  }
  void rebuild();
  void keyPressAnywhere(QKeyEvent *);
  void finalizeConnection();
  void startConnectionFromPin(QPointF);
  void startConnectionFromConnection(QPointF);
  static QRectF minRect();
  void resetSceneRect();
  void growSceneRect(QSet<int> const &eltids);
  QPointF pinPosition(int id, QString pin) const;
  bool undo();
  bool redo();
  void preact();
  void unact();
  QSet<int> selectedElements() const;
  void rotateElement(int id, int steps=1);
  void rotateSelection(int steps=1);
  void rotateElementOrSelection(int steps=1); // creates undo step
  void flipElement(int id);
  void flipSelection();
  void flipElementOrSelection(); // creates undo step
  void rebuildAsNeeded(CircuitMod const &cm);
  void rebuildAsNeeded(QSet<int> elts, QSet<int> cons);
  void backspace();
  void startSymbolDragIn(QString sym, QPointF sp);
  bool startSvgDragIn(QString fn, QPointF sp);
  QPointF moveDragIn(QPointF sp);
  void hideDragIn();
  bool importAndPlonk(QString filename, QPointF sp, bool merge=true);
public:
  inline Circuit const &circ() const { return schem.circuit(); }
  inline Circuit &circ() { return schem.circuit(); }
  inline SymbolLibrary const &lib() const { return schem.library(); }
  inline SymbolLibrary &lib() { return schem.library(); }
public:
  Scene *scene;
  Schem schem;
  QMap<int, class SceneElement *> elts;
  QMap<int, class SceneConnection *> conns;
  QPointF mousexy;
  HoverManager *hovermanager;
  QList<Circuit> undobuffer;
  QList< QSet<int> > undoselections;
  QList<Circuit> redobuffer;
  QList< QSet<int> > redoselections;
  ConnBuilder *connbuilder;
  FloatingSymbol *dragin;
  PartList *partlist;
  QGraphicsRectItem *rubberband;
  QPointF rbstart;
  QSet<int> prebandselection;
};

QRectF SceneData::minRect() {
  return QRectF(-1000, -1000, 2000, 2000);
}

QPointF SceneData::pinPosition(int id, QString pin) const {
  if (circ().elements.contains(id))
    return lib().upscale(Geometry(circ(), lib()).pinPosition(id, pin));
  else 
    return QPoint();
}

bool SceneData::undo() {
  if (undobuffer.isEmpty())
    return false;

  redobuffer << circ();
  redoselections << selectedElements();

  circ() = undobuffer.takeLast();
  rebuild();

  scene->clearSelection();
  for (int id: undoselections.takeLast())
    if (elts.contains(id))
      elts[id]->setSelected(true);

  return true;
}

bool SceneData::redo() {
  if (redobuffer.isEmpty())
    return false;

  undobuffer << circ();
  undoselections << selectedElements();

  circ() = redobuffer.takeLast();
  rebuild();

  scene->clearSelection();
  for (int id: redoselections.takeLast())
    if (elts.contains(id))
      elts[id]->setSelected(true);

  return true;
}

void SceneData::preact() {
  undobuffer << circ();
  undoselections << selectedElements();
  redobuffer.clear();
  redoselections.clear();
}

void SceneData::unact() {
  undobuffer.removeLast();
  undoselections.removeLast();
}

void SceneData::startConnectionFromPin(QPointF pos) {
  scene->clearSelection();
  connbuilder = new ConnBuilder(scene);
  scene->addItem(connbuilder);
  connbuilder->startFromPin(pos,
                            hovermanager->element(),
                            hovermanager->pin());
}

void SceneData::startConnectionFromConnection(QPointF pos) {
  scene->clearSelection();
  connbuilder = new ConnBuilder(scene);
  scene->addItem(connbuilder);
  connbuilder->startFromConnection(pos,
                                   hovermanager->connection(),
                                   hovermanager->segment());
}

void SceneData::rebuildAsNeeded(CircuitMod const &cm) {
  circ() = cm.circuit();
  rebuildAsNeeded(cm.affectedElements(), cm.affectedConnections());
  scene->emitCircuitChanged();
}

void SceneData::rebuildAsNeeded(QSet<int> eltids, QSet<int> conids) {
  for (int id: conids) {
    if (circ().connections.contains(id)) {
      if (conns.contains(id))
        conns[id]->rebuild();
      else
        conns[id] = new SceneConnection(scene, circ().connections[id]);
    } else if (conns.contains(id)) {
      delete conns[id];
      conns.remove(id);
    }
  }

  for (int id: eltids) {
    if (circ().elements.contains(id)) {
      if (elts.contains(id))
        elts[id]->rebuild();
      else
        elts[id] = new SceneElement(scene, circ().elements[id]);
    } else if (elts.contains(id)) {
      delete elts[id];
      elts.remove(id);
    }
  }

  growSceneRect(eltids);
  hovermanager->update();
  partlist->rebuild();
}

void SceneData::growSceneRect(QSet<int> const &eltids) {
  QRectF r0;
  for (int id: eltids)
    if  (elts.contains(id))
      r0 |= elts[id]->sceneBoundingRect();
  r0.adjust(-100, -100, 100, 100);
  scene->setSceneRect(r0 | scene->sceneRect());
}

void SceneData::resetSceneRect() {
  QRectF r0 = minRect();
  for (auto e: elts)
    r0 |= e->sceneBoundingRect();
  for (auto c: conns)
    r0 |= c->sceneBoundingRect();
  r0.adjust(-100, -100, 100, 100);
  scene->setSceneRect(r0);
}  

void SceneData::rotateElement(int id, int steps) {
  CircuitMod cm(circ(), lib());
  cm.rotateElement(id, steps);
  rebuildAsNeeded(cm);
}

void SceneData::rotateSelection(int steps) {
  CircuitMod cm(circ(), lib());
  cm.rotateElements(selectedElements(), steps);
  rebuildAsNeeded(cm);
}  

void SceneData::flipElement(int id) {
  CircuitMod cm(circ(), lib());
  cm.flipElement(id);
  rebuildAsNeeded(cm);
}

void SceneData::flipSelection() {
  CircuitMod cm(circ(), lib());
  cm.flipElements(selectedElements());
  rebuildAsNeeded(cm);
}  


Scene::~Scene() {
  delete d;
}
  
Scene::Scene(Schem const &schem, QObject *parent):
  QGraphicsScene(parent), d(new SceneData(this, schem)) {
  d->hovermanager = new HoverManager(this);
  d->partlist = new PartList(this);  
  // auto *test = new QGraphicsTextItem;
  // test->setHtml("Hello world");
  // test->setPos(QPointF(100, 50));
  // test->setTextInteractionFlags(Qt::TextEditorInteraction);
  // test->setFlags(QGraphicsItem::ItemIsFocusable);
  // addItem(test);
  d->rebuild();
}

/*
void Scene::setCircuit(Circuit const &c) {
  for (auto i: d->elts)
    delete i;
  d->elts.clear();
  for (auto i: d->conns)
    delete i;
  d->conns.clear();
  d->circ = c;
  d->rebuild();
}
*/

void SceneData::rebuild() {
  /* We should be able to do better than start afresh in general, but for now: */
  for (auto i: elts)
    delete i;
  elts.clear();
  for (auto i: conns)
    delete i;
  conns.clear();

  for (auto const &c: circ().elements) 
    elts[c.id] = new SceneElement(scene, c);
  
  for (auto const &c: circ().connections)
    conns[c.id] = new SceneConnection(scene, c);

  resetSceneRect();
  partlist->rebuild();
}

QPointF Scene::pinPosition(int eltid, QString pin) const {
  return d->pinPosition(eltid, pin);
}


SymbolLibrary const &Scene::library() const {
  return d->lib();
}

Schem const &Scene::schem() const {
  return d->schem;
}

Circuit const &Scene::circuit() const {
  return d->circ();
}

QSet<int> SceneData::selectedElements() const {
  QSet<int> selection;
  for (int id: elts.keys())
    if (elts[id]->isSelected())
      selection << id;
  return selection;
}

QSet<int> Scene::selectedElements() const {
  return d->selectedElements();
}

void Scene::selectElements(QSet<int> const &set) {
  QSet<int> old = selectedElements();
  for (int id: set-old)
    if (d->elts.contains(id))
      d->elts[id]->setSelected(true);
  for (int id: old-set)
    if (d->elts.contains(id))
      d->elts[id]->setSelected(false);
}

void Scene::tentativelyMoveSelection(QPoint delta, bool first,
				     bool nomagnet) {
  QSet<int> selection = selectedElements();
  if (first)
    d->hovermanager->formSelection(selection);
  delta = d->hovermanager->tentativelyMoveSelection(delta, nomagnet);
  QSet<int> internalcons = d->circ().connectionsIn(selection);
  QSet<int> fromcons = d->circ().connectionsFrom(selection) - internalcons;
  QSet<int> tocons = d->circ().connectionsTo(selection) - internalcons;

  for (int id: selection)
    d->elts[id]->temporaryTranslate(delta);
  for (int id: internalcons)
    d->conns[id]->temporaryTranslate(delta);
  for (int id: fromcons)
    d->conns[id]->temporaryTranslateFrom(delta);
  for (int id: tocons)
    d->conns[id]->temporaryTranslateTo(delta);
}

void Scene::moveSelection(QPoint delta, bool nomagnet) {
  delta = d->hovermanager->tentativelyMoveSelection(delta, nomagnet);
  d->hovermanager->doneDragging();

  QSet<int> selection = selectedElements();
  QSet<int> internalcons = d->circ().connectionsIn(selection);
  QSet<int> fromcons = d->circ().connectionsFrom(selection) - internalcons;
  QSet<int> tocons = d->circ().connectionsTo(selection) - internalcons;
  
  if (!delta.isNull()) {
    // must actually change circuit
    d->preact();
    Circuit origCirc(d->circ());
    CircuitMod cm(d->circ(), d->lib());
    for (int id: selection)
      cm.translateElement(id, delta);
    for (int id: internalcons)
      cm.translateConnection(id, delta);

    for (int id: fromcons)
      cm.reroute(id, origCirc);
    for (int id: tocons)
      cm.reroute(id, origCirc);

    for (int id: selection)
      cm.removeOverlappingJunctions(id);
    for (int id: fromcons)
      cm.adjustOverlappingConnections(id);
    for (int id: tocons)
      cm.adjustOverlappingConnections(id);

    // Just a little sanity check
    Circuit newcirc = cm.circuit();
    for (int id: selection)
      if (!newcirc.elements.contains(id))
	qDebug() << "Selected element" << origCirc.elements[id].report()
		 << "got deleted in move. This might be bad.";
    // End of sanity check

    // And now: Let's try to merge
    cm.mergeSelection(selection);
    
    d->rebuildAsNeeded(cm);

  } else {
    // restore stuff
    for (int id: selection)
      d->elts[id]->rebuild();
    for (int id: internalcons + fromcons + tocons)
      d->conns[id]->rebuild();
  }
}

void Scene::mousePressEvent(QGraphicsSceneMouseEvent *e) {
  d->mousexy = e->scenePos();
  d->hovermanager->setPrimaryPurpose((d->connbuilder ||
                                      e->modifiers() & Qt::ControlModifier)
                                     ? HoverManager::Purpose::Connecting
                                     : HoverManager::Purpose::Moving);
  d->hovermanager->update(e->scenePos());

  if (d->connbuilder) {
    d->connbuilder->mousePress(e);
    update();
  } else if (d->hovermanager->onPin()) {
    d->startConnectionFromPin(e->scenePos());
  } else if (d->hovermanager->onFakePin()) {
    d->startConnectionFromConnection(e->scenePos());
  } else if (d->hovermanager->onConnection()
	     && (e->modifiers() & Qt::ControlModifier)) {
    d->startConnectionFromConnection(e->scenePos());
  } else if (d->hovermanager->onElement()) {
    QGraphicsScene::mousePressEvent(e); // pass on to items
    int elt = d->hovermanager->element();
    if (e->modifiers() & Qt::ShiftModifier) {
      // toggle
      if (d->elts.contains(elt))
	d->elts[elt]->setSelected(!d->elts[elt]->isSelected());
    } else {
      // if not selected, select exclusively
      if (d->elts.contains(elt) && !d->elts[elt]->isSelected()) {
	clearSelection();
	d->elts[elt]->setSelected(true);
      }
    }
    d->rbstart = e->scenePos();
  } else if (d->hovermanager->onConnection()) {
    QGraphicsScene::mousePressEvent(e); // pass on to items
  } else if (itemAt(e->scenePos(), QTransform())) {
    QGraphicsScene::mousePressEvent(e); // pass on to items
  } else {
    if (!(e->modifiers() & Qt::ShiftModifier))
	clearSelection();
    d->rbstart = e->scenePos();
    d->prebandselection = selectedElements();
    d->rubberband = new QGraphicsRectItem;
    d->rubberband->setRect(QRectF(d->rbstart, d->rbstart));
    d->rubberband->setBrush(QBrush(QColor(0, 128, 255, 32)));
    d->rubberband->setBrush(QBrush(QColor(255, 255, 0, 64)));
    d->rubberband->setPen(QPen(Qt::NoPen));
    d->rubberband->setZValue(-1000);
    addItem(d->rubberband);
  }
}

void Scene::mouseMoveEvent(QGraphicsSceneMouseEvent *e) {
  d->mousexy = e->scenePos();
  auto purp = HoverManager::Purpose::Moving;
  if (d->connbuilder)
    purp = HoverManager::Purpose::Connecting;
  else if (e->buttons())
    // already doing something
    purp = HoverManager::Purpose::None;
  else if (e->modifiers() & Qt::ControlModifier)
    purp = HoverManager::Purpose::Connecting;
  
  d->hovermanager->setPrimaryPurpose(purp);
  d->hovermanager->update(e->scenePos());
  if (d->connbuilder) {
    d->connbuilder->mouseMove(e);
    update();
  } else if (d->rubberband) {
    QRectF rect = QRectF(d->rbstart, e->scenePos()).normalized();
    d->rubberband->setRect(rect);
    for (int id: d->elts.keys())
      d->elts[id]->setSelected(d->prebandselection.contains(id)
			       || rect.contains(d->elts[id]->sceneBoundingRect()));
  } else {
    QGraphicsScene::mouseMoveEvent(e);
  }
}

void Scene::mouseReleaseEvent(QGraphicsSceneMouseEvent *e) {
  d->mousexy = e->scenePos();
  d->hovermanager->setPrimaryPurpose((d->connbuilder ||
                                      e->modifiers() & Qt::ControlModifier)
                                     ? HoverManager::Purpose::Connecting
                                     : HoverManager::Purpose::Moving);
  d->hovermanager->update(e->scenePos());
  if (d->connbuilder) {
    d->connbuilder->mouseRelease(e);
    update();
    if (d->connbuilder->isComplete())
      d->finalizeConnection();
    d->hovermanager->setPrimaryPurpose((d->connbuilder ||
                                        e->modifiers() & Qt::ControlModifier)
                                       ? HoverManager::Purpose::Connecting
                                       : HoverManager::Purpose::Moving);
  } else if (d->rubberband) {
    delete d->rubberband;
    d->rubberband = 0;
  } else {
    QGraphicsScene::mouseReleaseEvent(e);
  }
}

void Scene::keyReleaseEvent(QKeyEvent *e) {
  // for whatever reason, releasing shift produces a key event with key==0
  // and modifiers() still containing shift. Useless.
  if (e->key()==Qt::Key_Shift
      && d->hovermanager->primaryPurpose() != HoverManager::Purpose::None)      
    d->hovermanager->setPrimaryPurpose(d->connbuilder
                                       ? HoverManager::Purpose::Connecting
                                       : HoverManager::Purpose::Moving);
  QGraphicsScene::keyReleaseEvent(e);
}

void Scene::keyPressEvent(QKeyEvent *e) {
  if (e->key()==Qt::Key_Shift
      && d->hovermanager->primaryPurpose() != HoverManager::Purpose::None)
    d->hovermanager->setPrimaryPurpose(HoverManager::Purpose::Connecting);

  if (d->connbuilder) {
    d->connbuilder->keyPress(e);
    if (d->connbuilder->isComplete())
      d->finalizeConnection();
    return;
  }
  
  if (focusItem()==0) {
    d->keyPressAnywhere(e);
  } else {
    QGraphicsScene::keyPressEvent(e);
  }
}

void SceneData::keyPressAnywhere(QKeyEvent *e) {
  QSet<int> ee = selectedElements();
  switch (e->key()) {
  // case Qt::Key_R:
  //   if (!ee.isEmpty()) {
  //     preact();
  //     rotateSelection();
  //   } else if (hovermanager->onElement()) {
  //     preact();
  //     rotateElement(hovermanager->element(),
  //                   (e->modifiers() & Qt::ShiftModifier) ? -1 : 1);
  //   }
  //   break;
  case Qt::Key_Delete:
    if (hovermanager->onElement()) {
      preact();
      CircuitMod cm(circ(), lib());
      cm.deleteElement(hovermanager->element());
      rebuildAsNeeded(cm);
    } else if (hovermanager->onConnection()) {
      preact();
      CircuitMod cm(circ(), lib());
      cm.deleteConnectionSegment(hovermanager->connection(),
                                 hovermanager->segment());
      rebuildAsNeeded(cm);
    } else if (!ee.isEmpty()) {
      preact();
      CircuitMod cm(circ(), lib());
      cm.deleteElements(ee);
      rebuildAsNeeded(cm);
    }
    break;
  case Qt::Key_Backspace:
    backspace();
    break;
  }
}

void SceneData::backspace() {
  SceneAnnotation *sa
    = dynamic_cast<SceneAnnotation *>(scene->itemAt(mousexy,
						    QTransform()));
  if (sa)
    sa->backspace();
}

//void Scene::makeUndoStep() {
//  d->preact();
//}

void Scene::undo() {
  d->undo();
}

void Scene::redo() {
  d->redo();
}

int Scene::elementAt(QPointF scenepos, int exclude) const {
  QGraphicsItem *it = itemAt(scenepos, QTransform());

  SceneElement *e = dynamic_cast<SceneElement *>(it);
  while (it && !e) {
    it = it->parentItem();
    e = dynamic_cast<SceneElement *>(it);
  }

  if (e && e->id() != exclude
      && e->boundingRect().contains(e->mapFromScene(scenepos)))
    return e->id();

  return -1;
}

static double L2(QPointF p) {
  return p.x()*p.x() + p.y()*p.y();
}

QString Scene::pinAt(QPointF scenepos, int elementId) const {
  if (!d->elts.contains(elementId))
    return PinID::NOPIN;
  QString sym = d->circ().elements[elementId].symbol();
  Symbol const &symbol = library().symbol(sym);
  double r = library().scale();
  for (auto p: symbol.pinNames())
    if (L2(scenepos - pinPosition(elementId, p)) <= 1.1*r*r)
      return p;
  return PinID::NOPIN;
}

int Scene::connectionAt(QPointF scenepos, int *segp) const {
  if (segp)
    *segp = -1;
  
  if (!itemAt(scenepos, QTransform()))
    return -1; // shortcut in case nothing there at all
  
 for (auto c: d->conns) {
   int seg = c->segmentAt(scenepos);
   if (seg>=0) {
     if (segp)
       *segp = seg;
     return c->id();
   }
 }
 
 return -1;
}

void SceneData::finalizeConnection() {
  if (!connbuilder->isAbandoned()) {
    preact();
    QList<int> cc;
    for (auto c: connbuilder->junctions()) {
      circ().insert(c);
      if (elts.contains(c.id))
        delete elts[c.id];
      elts[c.id] = new SceneElement(scene, c);
    }
    for (auto c: connbuilder->connections()) {
      circ().insert(c);
      if (conns.contains(c.id))
        delete conns[c.id];
      conns[c.id] = new SceneConnection(scene, c);
      cc << c.id;
    }

    CircuitMod cm(circ(), lib());
    for (int c: cc) 
      cm.simplifyConnection(c);
    for (int c: cc)
      cm.removeConnectionsEquivalentTo(c);
    for (int c: cc)
      cm.adjustOverlappingConnections(c);
    for (auto c: connbuilder->junctions())
      cm.removePointlessJunction(c.id);
    rebuildAsNeeded(cm);
  }

  delete connbuilder;
  connbuilder = 0;
}

QMap<int, class SceneElement *> const &Scene::elements() const {
  return d->elts;
}

QMap<int, class SceneConnection *> const &Scene::connections() const {
  return d->conns;
}

void Scene::modifyElementAnnotations(Element const &elt) {
  int id = elt.id;
  if (!elements().contains(id))
    return;

  d->preact();
  
  Element elt0 = d->circ().elements[id];
  elt0.copyAnnotationsFrom(elt);
  d->circ().insert(elt0);
  if (d->elts.contains(id))
    d->elts[id]->rebuild();
  d->partlist->rebuild();
  emitCircuitChanged();
}

void Scene::modifyConnection(int id, QPolygonF newpath) {
  if (!connections().contains(id))
    return;

  d->preact();
  
  Connection con(d->circ().connections[id]);
  if (con.fromId > 0)
    newpath.removeFirst();
  if (con.toId > 0)
    newpath.removeLast();
  con.via.clear();
  for (auto p: newpath)
    con.via << d->lib().downscale(p);
  d->circ().insert(con);

  CircuitMod cm(d->circ(), d->lib());
  cm.forceRebuildConnection(id);
  cm.adjustOverlappingConnections(id);
  cm.mergeConnection(id);
  d->rebuildAsNeeded(cm);

  unhover();
  rehover();
}

void Scene::copyToClipboard(bool cut) {
  QSet<int> elts;
  if (d->hovermanager->onElement())
    elts << d->hovermanager->element();
  else
    elts = selectedElements();

  if (elts.isEmpty())
      return;

  Circuit pp = d->circ().subset(elts);
  Clipboard::clipboard().store(pp, d->lib());
  if (cut) {
    d->preact();
    CircuitMod cm(d->circ(), d->lib());
    cm.deleteElements(elts);
    d->rebuildAsNeeded(cm);
  }
}

void Scene::pasteFromClipboard() {
  int mx = d->circ().maxId();
  Circuit pp = Clipboard::clipboard().retrieve();
  SymbolLibrary const &altlib = Clipboard::clipboard().library();
  QPoint cm = Geometry(pp, d->lib()).centerOfPinMass();
  pp.translate(d->lib().downscale(d->mousexy) - cm);
  int m2 = pp.renumber(mx + 1);
  if (m2<mx) {
    qDebug() << "nothing to paste";
    return;
  }

  d->preact();

  for (Element const &elt: pp.elements)
    if (!d->lib().contains(elt.symbol()))
      if (altlib.contains(elt.symbol()))
	d->lib().insert(altlib.symbol(elt.symbol()));
	  
  d->circ().merge(pp);
  d->rebuildAsNeeded(QSet<int>::fromList(pp.elements.keys()),
                     QSet<int>::fromList(pp.connections.keys()));
  clearSelection();
  for (int id: pp.elements.keys()) 
    if (d->elts.contains(id))
      d->elts[id]->setSelected(true);
}

void Scene::removeDangling() {
  d->preact();
  CircuitMod cm(d->circ(), d->lib());
  cm.removeAllDanglingOrInvalid();
  d->rebuildAsNeeded(cm);
}

void SceneData::startSymbolDragIn(QString sym, QPointF pos) {
  if (dragin)
    delete dragin;
  Symbol const &symbol = lib().symbol(sym);
  dragin = new FloatingSymbol(symbol);
  hovermanager->newDrag(symbol);
  moveDragIn(pos);
  scene->addItem(dragin);
}

QPointF SceneData::moveDragIn(QPointF scenepos) {
  if (!dragin)
    return scenepos;
  QPoint p = lib().downscale(scenepos - dragin->shiftedCenter());
  p = hovermanager->tentativelyMoveSelection(p);
  QPointF res = lib().upscale(p);
  dragin->setSymbolPosition(res);
  return res;
}

void SceneData::hideDragIn() {
  if (dragin)
    delete dragin;
  dragin = 0;
  hovermanager->doneDragging();
}

bool SceneData::startSvgDragIn(QString filename, QPointF pos) {
  Symbol symbol = Symbol::load(filename);
  qDebug() << "startSvgDragIn" << filename << pos << symbol.isValid();
  if (!symbol.isValid())
    return false;

  if (dragin)
    delete dragin;
  dragin = new FloatingSymbol(symbol);

  hovermanager->newDrag(symbol);
  moveDragIn(pos);
  scene->addItem(dragin);

  return true;
}

bool SceneData::importAndPlonk(QString filename, QPointF pos, bool merge) {
  Symbol symbol = Symbol::load(filename);
  if (!symbol.isValid())
    return false;
  lib().insert(symbol);

  QString name = symbol.name();
  
  QPoint pt = lib().downscale(pos);
  Element elt;
  if (name.startsWith("part:"))
    elt = Element::component(name.mid(5), pt);
  else if (name.startsWith("port:"))
    elt = Element::port(name.mid(5), pt);

  if (elt.isValid()) {
    CircuitMod cm(circ(), lib());
    cm.addElement(elt);
    if (merge) {
      QSet<int> ee;
      ee << elt.id;
      cm.mergeSelection(ee);
    }
    for (Element const &elt: circ().elements)
      if (elt.symbol()==name)
	cm.forceRebuildElement(elt.id);
    
    rebuildAsNeeded(cm);
  }

  return true;
}

void Scene::plonk(QString symbol, QPointF scenepos, bool merge) {
  clearSelection();
  QPoint pt = d->lib().downscale(scenepos);
  Element elt;
  if (symbol.startsWith("part:"))
    elt = Element::component(symbol.mid(5), pt);
  else if (symbol.startsWith("port:"))
    elt = Element::port(symbol.mid(5), pt);

  if (elt.isValid()) {
    elt.name = circuit().autoName(elt.symbol());
    elt.info.package = schem().packaging().recommendedPackage(elt.symbol());
    d->preact();
    CircuitMod cm(d->circ(), d->lib());
    cm.addElement(elt);
    if (merge) {
      QSet<int> ee;
      ee << elt.id;
      cm.mergeSelection(ee);
    }
    d->rebuildAsNeeded(cm);
  } 
}

void Scene::dragEnterEvent(QGraphicsSceneDragDropEvent *e) {
  d->hovermanager->update(e->scenePos());
  QMimeData const *md = e->mimeData();
  if (md->hasFormat("application/x-dnd-cschem")) {
    d->hovermanager->setPrimaryPurpose(HoverManager::Purpose::None);
    d->startSymbolDragIn(QString(md->data("application/x-dnd-cschem")),
		   e->scenePos());
    e->accept();
  } else if (md->hasUrls()) {
    QList<QUrl> urls = md->urls();
    QString fn;
    for (QUrl url: urls) {
      if (url.isLocalFile() && url.path().endsWith(".svg")) {
	fn = url.path();
	break;
      }
    }
    if (!fn.isEmpty()) {
      if (d->startSvgDragIn(fn, e->scenePos()))
	e->accept();
      else
	e->ignore();
    } else {
      e->ignore();
    }
  } else {
    e->ignore();
  }
}

void Scene::dragLeaveEvent(QGraphicsSceneDragDropEvent *e) {
  d->hovermanager->setPrimaryPurpose((d->connbuilder ||
                                      e->modifiers() & Qt::ControlModifier)
                                     ? HoverManager::Purpose::Connecting
                                     : HoverManager::Purpose::Moving);
  d->hovermanager->update(e->scenePos());
  d->hideDragIn();
}

void Scene::dragMoveEvent(QGraphicsSceneDragDropEvent *e) {
  d->hovermanager->update(e->scenePos());
  d->moveDragIn(e->scenePos());
  e->accept();
}

void Scene::dropEvent(QGraphicsSceneDragDropEvent *e) {
  d->hovermanager->setPrimaryPurpose((d->connbuilder ||
                                      e->modifiers() & Qt::ControlModifier)
                                     ? HoverManager::Purpose::Connecting
                                     : HoverManager::Purpose::Moving);
  d->hovermanager->update(e->scenePos());
  QPointF droppos = d->moveDragIn(e->scenePos());
  d->hideDragIn();

  QMimeData const *md = e->mimeData();
  if (md->hasFormat("application/x-dnd-cschem")) {
    plonk(QString(md->data("application/x-dnd-cschem")), droppos, true);
    e->accept();
  } else if (md->hasUrls()) {
    QList<QUrl> urls = md->urls();
    bool take = false;
    d->preact();
    clearSelection();
    for (QUrl url: urls)
      if (url.isLocalFile() && url.path().endsWith(".svg"))
        take = d->importAndPlonk(url.path(), e->scenePos(), true);
    if (take) {
        emit libraryChanged();
	e->accept();
    } else {
      d->unact();
      e->ignore();
    }
  } else {
    e->ignore();
  }
  // i'd like to set focus, but it doesn't happen somehow
}

void Scene::focusInEvent(QFocusEvent *e) {
  QGraphicsScene::focusInEvent(e);
}

void Scene::focusOutEvent(QFocusEvent *e) {
  QGraphicsScene::focusOutEvent(e);
}

void Scene::updateFromPartList(Element const &elt) {
  int id = elt.id;
  if (d->circ().elements.contains(id)) {
    Element &e(d->circ().elements[id]);
    e.name = elt.name;
    e.value = elt.value;
    e.info = elt.info;
  }
  if (d->elts.contains(id))
    d->elts[id]->rebuild();
  emitCircuitChanged();
}

void SceneData::rotateElementOrSelection(int dir) {
  QSet<int> ee = selectedElements();
  if (hovermanager->onElement()) {
    preact();
    rotateElement(hovermanager->element(), dir);
  } else if (!ee.isEmpty()) {
    preact();
    rotateSelection(dir);
  }
}

void Scene::rotate(int dir) {
  d->rotateElementOrSelection(dir);
}

void SceneData::flipElementOrSelection() {
  QSet<int> ee = selectedElements();
  if (hovermanager->onElement()) {
    preact();
    flipElement(hovermanager->element());
  } else if (!ee.isEmpty()) {
    preact();
    flipSelection();
  }
}

void Scene::flipx() {
  d->flipElementOrSelection();
}

void Scene::simplifySegment(int con, int seg) {
  d->hovermanager->unhover();
  d->preact();
  CircuitMod cm(d->circ(), d->lib());
  cm.simplifySegment(con, seg);
  d->rebuildAsNeeded(cm);
  d->hovermanager->update();
}

void Scene::unhover() {
  d->hovermanager->unhover();
}

void Scene::rehover() {
  d->hovermanager->update();
}

PartList *Scene::partlist() const {
  return d->partlist;
}

void Scene::emitCircuitChanged() {
  emit circuitChanged();
}

HoverManager *Scene::hoverManager() const {
  return d->hovermanager;
}

void Scene::clearSelection() {
  for (SceneElement *elt: elements())
    elt->setSelected(false);
}
