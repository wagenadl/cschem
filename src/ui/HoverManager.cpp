// HoverManager.cpp

#include "HoverManager.h"
#include "Scene.h"
#include "SceneConnection.h"
#include "SceneElement.h"
#include <QGraphicsEllipseItem>
#include <QDebug>
#include "Style.h"

class HoverManagerData {
public:
  HoverManagerData(Scene *scene): scene(scene) {
    elt = -1;
    pin = "-";
    con = -1;
    seg = -1;
    fakepin = false;
    isjunc = false;
    pinMarker = 0;
    primaryPurpose = HoverManager::Purpose::Moving;
    r = scene->library()->scale();
  }
  void ensurePinMarker();
  void update();
  bool onElement() const;
  bool onConnection() const;
  bool onPin() const;
  void unhover();
  void highlightPin();
  void highlightElement();
  void highlightSegment();
  void unhighlightPin();
  void unhighlightElement();
  void unhighlightSegment();
public:
  Scene *scene;
  QPointF pt;
  int elt;
  QString pin;
  QPointF pinpos;
  int con;
  int seg;
  bool fakepin;
  bool isjunc;
  QGraphicsEllipseItem *pinMarker;
  SceneElement::WeakPtr hoverElt;
  SceneConnection::WeakPtr hoverCon;
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

void HoverManagerData::highlightPin() {
    ensurePinMarker();
    pinMarker->setRect(QRectF(pinpos - QPointF(r, r), 2 * QSizeF(r, r)));
    int N = scene->circuit().connectionsOn(elt, pin).size();
    if (N==0)
      pinMarker->setBrush(Style::danglingColor());
    else
      pinMarker->setBrush(Style::pinHighlightColor());
}

void HoverManagerData::highlightElement() {
  auto const &elts = scene->elements();
  auto *e = elts[elt];
  if (hoverElt.data() != e) {
    if (hoverElt)
      hoverElt.data()->unhover();
    e->hover();
    hoverElt = e->weakref();
  }
}

void HoverManagerData::highlightSegment() {
  auto const &cons = scene->connections();
  auto *c = cons[con];
  if (hoverCon.data() != c) {
    if (hoverCon)
      hoverCon.data()->unhover();
    c->hover(seg);
    hoverCon = c->weakref();
  }
}

void HoverManagerData::unhighlightPin() {
  if (pinMarker) 
    pinMarker->setBrush(QColor(128, 128, 128, 0));
}

void HoverManagerData::unhighlightElement() {
  if (hoverElt)
    hoverElt.data()->unhover();
  hoverElt.clear();
}

void HoverManagerData::unhighlightSegment() {
  if (hoverCon)
    hoverCon.data()->unhover();
  hoverCon.clear();
}

void HoverManagerData::unhover() {
  unhighlightSegment();
  unhighlightElement();
  unhighlightPin();
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
    return con>0; // hmm.
  }
  return false; // not executed
}    

void HoverManagerData::update() {
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
    highlightPin();
    unhighlightSegment();
    unhighlightElement();
  } else if (onElement()) {
    highlightElement();
    unhighlightSegment();
    unhighlightPin();
  } else if (onConnection()) {
    Connection ccc(scene->circuit().connection(con));
    auto *lib = scene->library();
    qDebug() << "on connection" << con << seg
	     << ccc.danglingStart() << ccc.danglingEnd()
	     << ccc.via();
    if (seg==0
	&& ccc.danglingStart()
	&& !ccc.via().isEmpty()
	&& QLineF(pt, lib->upscale(ccc.via().first())).length() < r) {
      // near dangling start
      pinpos = scene->library()->upscale(ccc.via().first());
      fakepin = true;
      highlightPin();
      unhighlightSegment();
    } else if (seg==ccc.via().size()-(ccc.danglingStart()?2:1)
	       && ccc.danglingEnd()
	       && !ccc.via().isEmpty()
	       &&  QLineF(pt, lib->upscale(ccc.via().last())).length() < r) {
      pinpos = scene->library()->upscale(ccc.via().last());
      fakepin = true;
      highlightPin();
      unhighlightSegment();
    } else {
      fakepin = false;
      unhighlightPin();
      highlightSegment();
    }
    unhighlightElement();
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

bool HoverManager::onFakePin() const {
  return onConnection() && d->fakepin;
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
