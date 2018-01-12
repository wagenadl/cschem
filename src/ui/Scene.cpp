// Scene.cpp

#include "Scene.h"
#include <QDebug>
#include "SceneElement.h"
#include "SceneConnection.h"
#include <QGraphicsSceneMouseEvent>
#include "HoverManager.h"
#include "ConnBuilder.h"
#include "svg/CircuitMod.h"
#include "svg/Geometry.h"
#include "Clipboard.h"
#include <QMimeData>
#include "SceneAnnotation.h"
#include "FloatingPart.h"

class SceneData {
public:
  SceneData(Scene *scene, PartLibrary *lib):
    scene(scene), lib(lib) {
    hovermanager = 0;
    connbuilder = 0;
    dragin = 0;
  }
  void rebuild();
  void keyPressAnywhere(QKeyEvent *);
  void finalizeConnection();
  void startConnectionFromPin(QPointF);
  void startConnectionFromConnection(QPointF);
  QPointF pinPosition(int id, QString pin) const {
    if (circ.elements().contains(id))
      return lib->upscale(Geometry(circ, lib).pinPosition(id, pin));
    else 
      return QPoint();
  }
  bool undo() {
    if (undobuffer.isEmpty())
      return false;

    redobuffer << circ;
    redoselections << selectedElements();

    circ = undobuffer.takeLast();
    rebuild();

    scene->clearSelection();
    for (int id: undoselections.takeLast())
      if (elts.contains(id))
        elts[id]->setSelected(true);

    return true;
  }
  bool redo() {
    if (redobuffer.isEmpty())
      return false;

    undobuffer << circ;
    undoselections << selectedElements();

    circ = redobuffer.takeLast();
    rebuild();

    scene->clearSelection();
    for (int id: redoselections.takeLast())
      if (elts.contains(id))
        elts[id]->setSelected(true);

    return true;
  }
  void preact() {
    undobuffer << circ;
    undoselections << selectedElements();
    redobuffer.clear();
    redoselections.clear();
  }
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
  void startPartDragIn(QString sym, QPointF sp);
  bool startSvgDragIn(QString fn, QPointF sp);
  void moveDragIn(QPointF sp);
  void hideDragIn();
  bool importAndPlonk(QString filename, QPointF sp);
public:
  Scene *scene;
  PartLibrary *lib;
  Circuit circ;
  QMap<int, class SceneElement *> elts;
  QMap<int, class SceneConnection *> conns;
  QPointF mousexy;
  HoverManager *hovermanager;
  QList<Circuit> undobuffer;
  QList< QSet<int> > undoselections;
  QList<Circuit> redobuffer;
  QList< QSet<int> > redoselections;
  ConnBuilder *connbuilder;
  FloatingPart *dragin;
};

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
  circ = cm.circuit();
  rebuildAsNeeded(cm.affectedElements(), cm.affectedConnections());
}

void SceneData::rebuildAsNeeded(QSet<int> eltids, QSet<int> conids) {
  for (int id: conids) {
    if (circ.connections().contains(id)) {
      if (conns.contains(id))
        conns[id]->rebuild();
      else
        conns[id] = new SceneConnection(scene, circ.connection(id));
    } else if (conns.contains(id)) {
      delete conns[id];
      conns.remove(id);
    }
  }

  for (int id: eltids) {
    if (circ.elements().contains(id)) {
      if (elts.contains(id))
        elts[id]->rebuild();
      else
        elts[id] = new SceneElement(scene, circ.element(id));
    } else if (elts.contains(id)) {
      delete elts[id];
      elts.remove(id);
    }
  }

  if (!eltids.isEmpty())
    scene->annotationInternallyEdited(-1); // crude
  
  hovermanager->update();
}

void SceneData::rotateElement(int id, int steps) {
  CircuitMod cm(circ, lib);
  cm.rotateElement(id, steps);
  rebuildAsNeeded(cm);
}

void SceneData::rotateSelection(int steps) {
  CircuitMod cm(circ, lib);
  cm.rotateElements(selectedElements(), steps);
  rebuildAsNeeded(cm);
}  

void SceneData::flipElement(int id) {
  CircuitMod cm(circ, lib);
  cm.flipElement(id);
  rebuildAsNeeded(cm);
}

void SceneData::flipSelection() {
  CircuitMod cm(circ, lib);
  cm.flipElements(selectedElements());
  rebuildAsNeeded(cm);
}  


Scene::~Scene() {
  delete d;
}
  
Scene::Scene(PartLibrary *lib, QObject *parent):
  QGraphicsScene(parent) {
  d = new SceneData(this, lib);
  d->hovermanager = new HoverManager(this);
  // auto *test = new QGraphicsTextItem;
  // test->setHtml("Hello world");
  // test->setPos(QPointF(100, 50));
  // test->setTextInteractionFlags(Qt::TextEditorInteraction);
  // test->setFlags(QGraphicsItem::ItemIsFocusable);
  // addItem(test);
}

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


void SceneData::rebuild() {
  /* We should be able to do better than start afresh in general, but for now: */
  for (auto i: elts)
    delete i;
  elts.clear();
  for (auto i: conns)
    delete i;
  conns.clear();

  for (auto const &c: circ.elements()) 
    elts[c.id()] = new SceneElement(scene, c);
  
  for (auto const &c: circ.connections())
    conns[c.id()] = new SceneConnection(scene, c);
}

QPointF Scene::pinPosition(int partid, QString pin) const {
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
    Circuit origCirc(d->circ);
    CircuitMod cm(d->circ, d->lib);
    for (int id: selection)
      cm.translateElement(id, dd);
    for (int id: internalcons)
      cm.translateConnection(id, dd);

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
      if (!newcirc.elements().contains(id))
	qDebug() << "Selected element" << origCirc.element(id).report()
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
  qDebug() << "Scene: mousePress" << e->scenePos();
  d->mousexy = e->scenePos();
  d->hovermanager->setPrimaryPurpose((d->connbuilder ||
                                      e->modifiers() & Qt::ShiftModifier)
                                     ? HoverManager::Purpose::Connecting
                                     : HoverManager::Purpose::Moving);
  d->hovermanager->update(e->scenePos());

  if (d->connbuilder) {
    d->connbuilder->mousePress(e);
    e->accept();
    update();
  } else {
    if (d->hovermanager->onPin()) {
      d->startConnectionFromPin(e->scenePos());
      e->accept();
    } else if (d->hovermanager->onFakePin()) {
      d->startConnectionFromConnection(e->scenePos());
      e->accept();
    } else if (d->hovermanager->onConnection()
               && (e->modifiers() & Qt::ShiftModifier)) {
      d->startConnectionFromConnection(e->scenePos());
      e->accept();
    } else {
      QGraphicsScene::mousePressEvent(e);
    }    
  }
}

void Scene::mouseMoveEvent(QGraphicsSceneMouseEvent *e) {
  d->mousexy = e->scenePos();
  d->hovermanager->setPrimaryPurpose((d->connbuilder ||
                                      e->modifiers() & Qt::ShiftModifier)
                                     ? HoverManager::Purpose::Connecting
                                     : e->buttons()
                                     ? HoverManager::Purpose::None
                                     : HoverManager::Purpose::Moving);
  d->hovermanager->update(e->scenePos());
  if (d->connbuilder) {
    d->connbuilder->mouseMove(e);
    update();
  } else {
    QGraphicsScene::mouseMoveEvent(e);
  }
}

void Scene::mouseReleaseEvent(QGraphicsSceneMouseEvent *e) {
  qDebug() << "Scene: mouseRelease" << e->scenePos();
  d->mousexy = e->scenePos();
  d->hovermanager->setPrimaryPurpose((d->connbuilder ||
                                      e->modifiers() & Qt::ShiftModifier)
                                     ? HoverManager::Purpose::Connecting
                                     : HoverManager::Purpose::Moving);
  d->hovermanager->update(e->scenePos());
  if (d->connbuilder) {
    d->connbuilder->mouseRelease(e);
    update();
    if (d->connbuilder->isComplete())
      d->finalizeConnection();
    d->hovermanager->setPrimaryPurpose((d->connbuilder ||
                                        e->modifiers() & Qt::ShiftModifier)
                                       ? HoverManager::Purpose::Connecting
                                       : HoverManager::Purpose::Moving);
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
  qDebug() << "scene::keypress" << focusItem();
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
    qDebug() << "delete" << ee.isEmpty() << hovermanager->onElement()
	     << hovermanager->element();
    if (!ee.isEmpty()) {
      preact();
      CircuitMod cm(circ, lib);
      cm.deleteElements(ee);
      rebuildAsNeeded(cm);
    } else if (hovermanager->onElement()) {
      preact();
      CircuitMod cm(circ, lib);
      cm.deleteElement(hovermanager->element());
      rebuildAsNeeded(cm);
    } else if (hovermanager->onConnection()) {
      preact();
      CircuitMod cm(circ, lib);
      cm.deleteConnectionSegment(hovermanager->connection(),
                                 hovermanager->segment());
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

void Scene::makeUndoStep() {
  d->preact();
}

void Scene::undo() {
  d->undo();
}

void Scene::redo() {
  d->redo();
}

int Scene::elementAt(QPointF scenepos) const {
  if (!itemAt(scenepos, QTransform()))
    return -1; // shortcut in case nothing there at all
  
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
      circ.insert(c);
      if (elts.contains(c.id()))
        delete elts[c.id()];
      elts[c.id()] = new SceneElement(scene, c);
    }
    for (auto c: connbuilder->connections()) {
      circ.insert(c);
      if (conns.contains(c.id()))
        delete conns[c.id()];
      conns[c.id()] = new SceneConnection(scene, c);
      cc << c.id();
    }

    CircuitMod cm(circ, lib);
    for (int c: cc) 
      cm.simplifyConnection(c);
    for (int c: cc)
      qDebug() << "Connection" << circ.connection(c).report();
    for (int c: cc)
      cm.removeConnectionsEquivalentTo(c);
    for (int c: cc)
      cm.adjustOverlappingConnections(c);
    for (auto c: connbuilder->junctions())
      cm.removePointlessJunction(c.id());
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

void Scene::modifyConnection(int id, QPolygonF newpath) {
  qDebug() << newpath << id;
  if (!connections().contains(id))
    return;

  d->preact();
  
  Connection con(d->circ.connection(id));
  if (con.fromId() > 0)
    newpath.removeFirst();
  if (con.toId() > 0)
    newpath.removeLast();
  QPolygon pp;
  for (auto p: newpath)
    pp << d->lib->downscale(p);
  con.setVia(pp);
  d->circ.insert(con);
  d->conns[id]->rebuild();
  qDebug() << "rebuilt";

  CircuitMod cm(d->circ, d->lib);
  if (cm.adjustOverlappingConnections(id))
    d->rebuildAsNeeded(cm);
}

void Scene::copyToClipboard(bool cut) {
  QSet<int> elts = selectedElements();
  if (elts.isEmpty()) {
    if (d->hovermanager->onElement())
      elts << d->hovermanager->element();
    else
      return;
  }

  Circuit pp = d->circ.subset(elts);
  Clipboard::clipboard().store(pp);
  if (cut) {
    d->preact();
    CircuitMod cm(d->circ, d->lib);
    cm.deleteElements(elts);
    d->rebuildAsNeeded(cm);
  }
}

void Scene::pasteFromClipboard() {
  int mx = d->circ.maxId();
  Circuit pp = Clipboard::clipboard().retrieve();
  QPoint cm = Geometry(pp, d->lib).centerOfPinMass();
  pp.translate(d->lib->downscale(d->mousexy) - cm);
  int m2 = pp.renumber(mx + 1);
  if (m2<mx) {
    qDebug() << "nothing to paste";
    return;
  }
  d->preact();
  d->circ += pp;
  d->rebuildAsNeeded(QSet<int>::fromList(pp.elements().keys()),
                     QSet<int>::fromList(pp.connections().keys()));
  clearSelection();
  for (int id: pp.elements().keys()) 
    if (d->elts.contains(id))
      d->elts[id]->setSelected(true);
}

void Scene::removeDangling() {
  d->preact();
  CircuitMod cm(d->circ, d->lib);
  cm.removeAllDanglingOrInvalid();
  d->rebuildAsNeeded(cm);
}

void SceneData::startPartDragIn(QString symbol, QPointF pos) {
  if (dragin)
    delete dragin;
  dragin = new FloatingPart(lib->part(symbol), pos);
  scene->addItem(dragin);
  moveDragIn(pos);
}

void SceneData::moveDragIn(QPointF scenepos) {
  QPointF pt = lib->nearestGrid(scenepos);
  if (dragin)
    dragin->setPartPosition(pt);
}

void SceneData::hideDragIn() {
  if (dragin)
    delete dragin;
  dragin = 0;
}

bool SceneData::startSvgDragIn(QString filename, QPointF pos) {
  PartLibrary pl(filename);
  if (pl.partNames().isEmpty())
    return false;
  Part part = pl.part(pl.partNames().first());
  if (dragin)
    delete dragin;
  dragin = new FloatingPart(part, pos);
  scene->addItem(dragin);
  moveDragIn(pos);
  return true;
}

bool SceneData::importAndPlonk(QString filename, QPointF pos) {
  qDebug() << "import" << filename << "at" << pos << "NYI";
  PartLibrary pl(filename);
  if (pl.partNames().isEmpty())
    return false;
  lib->merge(filename); // I should allow merging two libraries directly
  // or else I should make merge return a list of parts successfully merged.

  scene->clearSelection();
  QString symbol = pl.partNames().first();
  
  QPoint pt = lib->downscale(pos);
  Element elt;
  if (symbol.startsWith("part:"))
    elt = Element::component(symbol.mid(5), pt);
  else if (symbol.startsWith("port:"))
    elt = Element::port(symbol.mid(5), pt);

  if (elt.isValid()) {
    preact();
    CircuitMod cm(circ, lib);
    cm.addElement(elt);
    rebuildAsNeeded(cm);
  }
  return true;
}

void Scene::plonk(QString symbol, QPointF scenepos, bool merge) {
  clearSelection();
  qDebug() << "plonk" << symbol << scenepos;
  QPoint pt = d->lib->downscale(scenepos);
  Element elt;
  if (symbol.startsWith("part:"))
    elt = Element::component(symbol.mid(5), pt);
  else if (symbol.startsWith("port:"))
    elt = Element::port(symbol.mid(5), pt);

  if (elt.isValid()) {
    d->preact();
    CircuitMod cm(d->circ, d->lib);
    cm.addElement(elt);
    if (merge) {
      QSet<int> ee;
      ee << elt.id();
      cm.mergeSelection(ee);
    }
    d->rebuildAsNeeded(cm);
  } 
}

void Scene::dragEnterEvent(QGraphicsSceneDragDropEvent *e) {
  d->hovermanager->update(e->scenePos());
  QMimeData const *md = e->mimeData();
  qDebug() << "drag enter" << md->formats();
  if (md->hasFormat("application/x-dnd-cschem")) {
    d->hovermanager->setPrimaryPurpose(HoverManager::Purpose::None);
    d->startPartDragIn(QString(md->data("application/x-dnd-cschem")),
		   e->scenePos());
    e->accept();
  } else if (md->hasUrls()) {
    QList<QUrl> urls = md->urls();
    qDebug() << "urls" << urls;
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
                                      e->modifiers() & Qt::ShiftModifier)
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
                                      e->modifiers() & Qt::ShiftModifier)
                                     ? HoverManager::Purpose::Connecting
                                     : HoverManager::Purpose::Moving);
  d->hovermanager->update(e->scenePos());
  d->hideDragIn();

  QMimeData const *md = e->mimeData();
  qDebug() << "drop" << md->formats();
  if (md->hasFormat("application/x-dnd-cschem")) {
    plonk(QString(md->data("application/x-dnd-cschem")), e->scenePos(), true);
    e->accept();
  } else if (md->hasUrls()) {
    QList<QUrl> urls = md->urls();
    qDebug() << "urls" << urls;
    bool take = false;
    for (QUrl url: urls)
      if (url.isLocalFile() && url.path().endsWith(".svg"))
        take = d->importAndPlonk(url.path(), e->scenePos());
    if (take)
      e->accept();
    else
      e->ignore();
  } else {
    e->ignore();
  }
}

void Scene::focusInEvent(QFocusEvent *e) {
  qDebug() << "Scene::focusin";
  QGraphicsScene::focusInEvent(e);
}

void Scene::focusOutEvent(QFocusEvent *e) {
  qDebug() << "Scene::focusout";
  QGraphicsScene::focusOutEvent(e);
}

void Scene::annotationInternallyEdited(int id) {
  qDebug() <<"internally edited" << id;
  emit annotationEdited(id);
}

void Scene::setComponentValue(int id, QString val) {
  qDebug() <<"externally edited" << id;
  if (d->circ.elements().contains(id)) {
    Element elt = d->circ.element(id);
    elt.setValue(val);
    d->circ.insert(elt);
  }
  if (d->elts.contains(id))
    d->elts[id]->rebuild();
}

void SceneData::rotateElementOrSelection(int dir) {
  QSet<int> ee = selectedElements();
  if (!ee.isEmpty()) {
    preact();
    rotateSelection(dir);
  } else if (hovermanager->onElement()) {
    preact();
    rotateElement(hovermanager->element(), dir);
  }
}

void Scene::rotate(int dir) {
  d->rotateElementOrSelection(dir);
}

void SceneData::flipElementOrSelection() {
  QSet<int> ee = selectedElements();
  if (!ee.isEmpty()) {
    preact();
    flipSelection();
  } else if (hovermanager->onElement()) {
    preact();
    flipElement(hovermanager->element());
  }
}

void Scene::flipx() {
  d->flipElementOrSelection();
}

void Scene::simplifySegment(int con, int seg) {
  d->hovermanager->unhover();
  d->preact();
  CircuitMod cm(d->circ, d->lib);
  cm.simplifySegment(con, seg);
  d->rebuildAsNeeded(cm);
  d->hovermanager->update();
}
