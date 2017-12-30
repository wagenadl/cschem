// HoverManager.cpp

#include "HoverManager.h"
#include "Scene.h"
#include "SceneConnection.h"
#include "SceneElement.h"
#include <QGraphicsEllipseItem>
#include <QDebug>

class HoverManagerData {
public:
  HoverManagerData(Scene *scene): scene(scene) {
    elt = -1;
    pin = "-";
    con = -1;
    seg = -1;
    isjunc = false;
    pinMarker = 0;
    primaryPurpose = HoverManager::Purpose::Moving;
    r = scene->library()->scale();
    hoverElt = 0;
    hoverCon = 0;
  }
  void ensurePinMarker();
  void update();
  bool onElement() const;
  bool onConnection() const;
  bool onPin() const;
  void unhover();
public:
  Scene *scene;
  QPointF pt;
  int elt;
  QString pin;
  QPointF pinpos;
  int con;
  int seg;
  bool isjunc;
  QGraphicsEllipseItem *pinMarker;
  SceneElement *hoverElt;
  SceneConnection *hoverCon;
  HoverManager::Purpose primaryPurpose;
  double r;
};

void HoverManagerData::ensurePinMarker() {
  if (pinMarker)
    return;
  pinMarker = new QGraphicsEllipseItem;
  pinMarker->setBrush(QBrush(QColor(128, 128, 128, 0))); // initially invisible
  pinMarker->setPen(QPen(Qt::NoPen));
  pinMarker->setZValue(-10);
  scene->addItem(pinMarker);
}

void HoverManagerData::unhover() {
  if (hoverCon)
    hoverCon->unhover();
  hoverCon = 0;
  if (hoverElt)
    hoverElt->unhover();
  hoverElt = 0;
  if (pinMarker) 
    pinMarker->setBrush(QColor(128, 128, 128, 0));
}

bool HoverManagerData::onPin() const {
  switch (primaryPurpose) {
  case HoverManager::Purpose::None:
    return false;
  case HoverManager::Purpose::Moving:
    if (elt>0)
      return pin!="-" && !isjunc;
    else
      return false;
  case HoverManager::Purpose::Connecting:
    return (elt>0 && pin!="-"); // IN NEAR FUTURE: || con>0;
  }
  return false; // not executed
}

bool HoverManagerData::onElement() const {
  switch (primaryPurpose) {
  case HoverManager::Purpose::None:
    return false;
  case HoverManager::Purpose::Moving:
    if (elt>0)
      return pin=="-" || isjunc;
    else
      return false;
  case HoverManager::Purpose::Connecting:
    return false;
  }
  return false; // not executed
}
    
bool HoverManagerData::onConnection() const {
  switch (primaryPurpose) {
  case HoverManager::Purpose::None:
    return false;
  case HoverManager::Purpose::Moving:
    return con>0;
  case HoverManager::Purpose::Connecting:
    return false; // hmm.
  }
  return false; // not executed
}    

void HoverManagerData::update() {
  qDebug() << "HMD:update";
  
  auto const &elts = scene->elements();
  auto const &cons = scene->connections();
  //  auto const &lib = scene->library();
  auto const &circ = scene->circuit();

  if (elt>0)
    if (!elts.contains(elt))
      elt = -1;
  if (elt>0)
    if (!elts[elt]->boundingRect().contains(elts[elt]->mapFromScene(pt)))
      elt = -1;
  if (elt<0)
    elt = scene->elementAt(pt);

  if (elt>0)
    pin = scene->pinAt(pt, elt);
  else
    pin = "-";

  if (elt>0)
    isjunc = circ.element(elt).type() == Element::Type::Junction;
  else
    isjunc = false;

  if (con>0)
    if (!cons.contains(con))
      con = -1;
  if (con>0) {
    seg = cons[con]->segmentAt(pt);
    if (seg<0)
      con = -1;
  }
  if (con<0)
    con = scene->connectionAt(pt, &seg);

  // we may be doing a lot of double work here, but if it doesn't flicker,
  // I don't think it's a problem
  if (onPin()) {
    pinpos = scene->pinPosition(elt, pin);
    ensurePinMarker();
    pinMarker->setRect(QRectF(pinpos - QPointF(r, r), 2 * QSizeF(r, r)));
    int N = scene->circuit().connectionsOn(elt, pin).size();
    if (N==0)
      pinMarker->setBrush(QColor(255, 64, 0));
    else
      pinMarker->setBrush(QColor(0, 255, 128));
    if (hoverCon)
      hoverCon->unhover();
    hoverCon = 0;
    if (hoverElt)
      hoverElt->unhover();
    hoverElt = 0;
  } else if (onElement()) {
    auto *e = elts[elt];
    if (hoverElt != e) {
      if (hoverElt)
        hoverElt->unhover();
      e->hover();
      hoverElt = e;
    }
    if (hoverCon)
      hoverCon->unhover();
    hoverCon = 0;
    if (pinMarker) 
      pinMarker->setBrush(QColor(128, 128, 128, 0));
  } else if (onConnection()) {
    auto *c = cons[con];
    if (hoverCon != c) {
      if (hoverCon)
        hoverCon->unhover();
      c->hover(seg);
      hoverCon = c;
    }
    if (hoverElt)
      hoverElt->unhover();
    hoverElt = 0;
    if (pinMarker) 
      pinMarker->setBrush(QColor(128, 128, 128, 0));
  } else {
    unhover();
  }
}


HoverManager::HoverManager(Scene *scene): d(new HoverManagerData(scene)) {
}

HoverManager::~HoverManager() {
  delete d;
}

void HoverManager::setPrimaryPurpose(HoverManager::Purpose purpose) {
  d->primaryPurpose = purpose;
  d->update();
}

HoverManager::Purpose HoverManager::primaryPurpose() const {
  return d->primaryPurpose;
}


void HoverManager::update(QPointF pt) {
  d->pt = pt;
  d->update();
}

bool HoverManager::onElement() const {
  return d->onElement();
}

bool HoverManager::onConnection() const {
  return d->onConnection();
}

bool HoverManager::onPin() const {
  return d->onPin();
}

int HoverManager::element() const {
  return (onElement() || onPin()) ? d->elt : -1;
}

QString HoverManager::pin() const {
  return onPin() ? d->pin : "-";
}

int HoverManager::connection() const {
  return onConnection() ? d->con : -1;
}

int HoverManager::segment() const {
  return onConnection() ? d->seg : -1;
}

void HoverManager::unhover() {
  d->unhover();
}
