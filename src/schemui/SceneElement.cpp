// SceneElement.cpp

#include "SceneElement.h"
#include "SceneElementData.h"
#include "qt/SvgItem.h"
#include <QGraphicsEllipseItem>
#include "circuit/Element.h"
#include "Scene.h"
#include <QDebug>
#include <QGraphicsColorizeEffect>
#include <QGraphicsSceneMouseEvent>
#include "SceneAnnotation.h"
#include "Style.h"
#include <QSvgRenderer>
#include <QPainter>
#include "circuit/PartNumbering.h"
#include "svg/Geometry.h"

SceneElementData::~SceneElementData() {
}

void SceneElementData::markHover() {
  if (hover && !nhover && !vhover) {
    auto *ef = new QGraphicsColorizeEffect;
    ef->setColor(element->parentItem()->isSelected()
		 ? Style::selectedElementHoverColor()
		 : Style::hoverColor());
    element->setGraphicsEffect(ef);
  } else {
    element->setGraphicsEffect(0);    
  }
}  

void SceneElementData::removeName() {
  scene->makeUndoStep();
  Circuit &circ = scene->circuit(); 
  Element elt(circ.element(id));
  elt.setNameVisible(false);
  circ.insert(elt);
  if (name)
    name->hide();
}

void SceneElementData::removeValue() {
  scene->makeUndoStep();
  Circuit &circ = scene->circuit(); 
  Element elt(circ.element(id));
  elt.setValueVisible(false);
  circ.insert(elt);
  if (value)
    value->hide();
}

void SceneElementData::moveValue(QPointF delta) {
  if (delta.isNull())
    return;
  scene->makeUndoStep();
  Circuit &circ = scene->circuit(); 
  Element elt(circ.element(id));
  elt.setValuePos(elt.valuePos() + delta.toPoint());
  circ.insert(elt);
}

void SceneElementData::moveName(QPointF delta) {
  if (delta.isNull())
    return;
  scene->makeUndoStep();
  Circuit &circ = scene->circuit(); 
  Element elt(circ.element(id));
  elt.setNamePos(elt.namePos() + delta.toPoint());
  circ.insert(elt);
}

void SceneElementData::setNameText() {
  // Called when escape pressed
  Circuit &circ = scene->circuit(); 
  Element const &elt(circ.element(id));
  QString txt = elt.name();
  if (txt.isEmpty()) {
    name->setHtml("<i>nn</i>");
    name->setDefaultTextColor(Style::faintColor());
  } else {
    name->setHtml(PartNumbering::nameToHtml(txt));
    name->setDefaultTextColor(Style::textColor());
  }
  if (name->hasFocus())
    name->clearFocus();
}

void SceneElementData::getNameText() {
  // Called when return pressed
  scene->makeUndoStep();
  Circuit &circ = scene->circuit(); 
  Element elt(circ.element(id));
  QString txt = name->toPlainText();
  if (txt == "nn")
    txt = "";
  if (txt.isEmpty())
    txt = circ.autoName(elt.symbol());
  elt.setName(txt);
  circ.insert(elt);
  setNameText();
  scene->annotationInternallyEdited(elt.id());
}

void SceneElementData::setValueText() {
  Circuit &circ = scene->circuit(); 
  Element const &elt(circ.element(id));
  QString txt = elt.value();
  if (txt.isEmpty()) {
    value->setHtml("<i>value</i>");
    value->setDefaultTextColor(Style::faintColor());
  } else {
    value->setPlainText(txt);
    value->setDefaultTextColor(Style::textColor());
  }
  if (value->hasFocus())
    value->clearFocus();
}

void SceneElementData::getValueText() {
  scene->makeUndoStep();
  Circuit &circ = scene->circuit(); 
  Element elt(circ.element(id));
  QString txt = value->toPlainText();
  if (txt == "value")
    txt = "";
  txt = PartNumbering::prettyValue(txt, elt.name());
  elt.setValue(txt);
  circ.insert(elt);
  setValueText();
  scene->annotationInternallyEdited(elt.id());
}

SceneElement::SceneElement(class Scene *parent, Element const &elt):
  d(new SceneElementData) {
  //
  d->scene = parent;
  d->id = elt.id();
  d->sym = elt.symbol();

  SymbolLibrary const *lib = d->scene->library();
  Symbol const &symbol = lib->symbol(d->sym);
  if (!symbol.isValid())
    qDebug() << "Cannot find svg for symbol" << d->sym;

  d->element = new SvgItem(this);
  d->element->setRenderer(symbol.renderer());
  
  parent->addItem(this);
  rebuild();

  if (d->sym == "junction")
    setZValue(20);
  else
    setZValue(10);
  
  setFlag(ItemIsMovable);
  setFlag(ItemIsSelectable);
}

SceneElement::~SceneElement() {
  // delete d;
}

// static double L2(QPointF p) {
//   return p.x()*p.x() + p.y()*p.y();
// }

void SceneElement::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *) {
  Element elt = d->scene->circuit().element(d->id);
  if (elt.type() == Element::Type::Component
      || elt.type() == Element::Type::Port) {
    elt.setNameVisible(true);
    if (elt.type() == Element::Type::Component)
      elt.setValueVisible(true);
    d->scene->circuit().insert(elt);
    rebuild();
  }
}

void SceneElement::mousePressEvent(QGraphicsSceneMouseEvent *e) {
  d->dragmoved = false;
  Circuit const &circ = d->scene->circuit();
  SymbolLibrary const *lib = d->scene->library();
  d->delta0 = circ.element(d->id).position() - lib->downscale(e->scenePos());
  qDebug() << "Element Mouse press" << e->pos() << pos();
  QGraphicsItem::mousePressEvent(e);
}

void SceneElement::mouseMoveEvent(QGraphicsSceneMouseEvent *e) {
  auto const &lib = d->scene->library();
  auto const &circ = d->scene->circuit();
  QPoint newpos = lib->downscale(e->scenePos()) + d->delta0;
  QPoint oldpos = circ.element(d->id).position();
  qDebug() << "Mouse move" << newpos << oldpos;
  QGraphicsItem::mouseMoveEvent(e);
  
  if (d->dragmoved || newpos != oldpos) {
    qDebug() << "Position changed" << newpos << oldpos;
    d->scene->tentativelyMoveSelection(newpos - oldpos, !d->dragmoved);
    d->dragmoved = true;
  }
}

void SceneElement::mouseReleaseEvent(QGraphicsSceneMouseEvent *e) {
  auto const &lib = d->scene->library();
  auto const &circ = d->scene->circuit();
  QPoint newpos = lib->downscale(e->scenePos()) + d->delta0;
  QPoint oldpos = circ.element(d->id).position();
  qDebug() << "Mouse release" << newpos << oldpos;
  QGraphicsItem::mouseReleaseEvent(e);
  if (d->dragmoved || newpos != oldpos) {
    qDebug() << "Position changed";
    d->scene->moveSelection(newpos - oldpos);
  }
}

void SceneElement::temporaryTranslate(QPoint delta) {
  Circuit const &circ = d->scene->circuit();
  Element elt(circ.element(d->id));
  SymbolLibrary const *lib = d->scene->library();

  setPos(lib->upscale(elt.position() + delta));
}

void SceneElement::rebuild() {
  // Reconstruct element
  Circuit const &circ = d->scene->circuit();
  Element elt(circ.element(d->id));
  SymbolLibrary const *lib = d->scene->library();
  Symbol const &symbol = lib->symbol(elt.symbol());

  prepareGeometryChange();

  d->element->setRenderer(symbol.renderer());
  
  QPointF orig = symbol.bbOrigin();
  QTransform xf;
  xf.translate(orig.x(), orig.y());
  xf.rotate(elt.rotation()*-90);
  if (elt.isFlipped())
    xf.scale(-1, 1);
  xf.translate(-orig.x(), -orig.y());
  d->element->setTransform(xf);
  d->element->setPos(symbol.shiftedBBox().topLeft());

  setPos(lib->scale() * elt.position());

  // origin for labels
  QRectF bb = symbol.shiftedBBox();
  QPointF p0 = bb.center();
  QTransform xfl;
  xfl.rotate(elt.rotation()*-90);
  if (elt.isFlipped())
    xfl.scale(-1, 1);
  p0 = xfl.map(p0);
  bb = xfl.mapRect(bb);
  
  // Reconstruct name
  if (elt.isNameVisible()) {
    qDebug() << "name visible on" << elt.report();
    if (d->name) {
      d->name->show();
    } else {
      d->name = new SceneAnnotation(d->scene->library()->scale(), this);
      QObject::connect(d->name, SIGNAL(returnPressed()),
		       d.data(), SLOT(getNameText()));
      QObject::connect(d->name, SIGNAL(escapePressed()),
		       d.data(), SLOT(setNameText()));
      QObject::connect(d->name, SIGNAL(moved(QPointF)),
		       d.data(), SLOT(moveName(QPointF)));
      QObject::connect(d->name, SIGNAL(removalRequested()),
		       d.data(), SLOT(removeName()));
      QObject::connect(d->name, SIGNAL(hovering(bool)),
                       d.data(), SLOT(nameHovering(bool)));
    }
    QPoint p = elt.namePos();
    if (p.isNull()) {
      QPointF bl = bb.bottomLeft() - p0;
      p = (bl + lib->upscale(QPoint(4, 3))).toPoint();
      elt.setNamePos(p);
      d->scene->circuit().insert(elt);
    }
    d->name->setBaseline(p + p0);
    d->setNameText();
    if (elt.name().isEmpty()) {
      d->getNameText(); // automated magic
      elt = circ.element(d->id);
    }
  } else if (d->name) {
    d->name->hide();
  }
  
  // Reconstruct value
  if (elt.isValueVisible()) {
    if (d->value) {
      d->value->show();
    } else {
      d->value = new SceneAnnotation(d->scene->library()->scale(), this);
      QObject::connect(d->value, SIGNAL(returnPressed()),
		       d.data(), SLOT(getValueText()));
      QObject::connect(d->value, SIGNAL(escapePressed()),
		       d.data(), SLOT(setValueText()));
      QObject::connect(d->value, SIGNAL(moved(QPointF)),
		       d.data(), SLOT(moveValue(QPointF)));
      QObject::connect(d->value, SIGNAL(removalRequested()),
		       d.data(), SLOT(removeValue()));
      QObject::connect(d->name, SIGNAL(hovering(bool)),
                       d.data(), SLOT(valueHovering(bool)));
    }
    QPoint p = elt.valuePos();
    if (p.isNull()) {
      QPointF bl = bb.bottomLeft() - p0;
      p = (bl + lib->upscale(QPoint(4, 6))).toPoint();
      elt.setValuePos(p);
      d->scene->circuit().insert(elt);
    }
    d->value->setBaseline(p + p0);
    d->setValueText();
  } else if (d->value) {
    d->value->hide();
  }
}

Scene *SceneElement::scene() {
  return d->scene;
}

int SceneElement::id() const {
  return d->id;
}


void SceneElement::hover() {
  if (!d->hover) {
    d->hover = true;
    d->markHover();
  }
}

void SceneElement::unhover() {
  if (d->hover) {
    d->hover = false;
    d->markHover();
  }
}

void SceneElementData::nameHovering(bool h) {
  nhover = h;
  markHover();
}

void SceneElementData::valueHovering(bool h) {
  vhover = h;
  markHover();
}

SceneElement::WeakPtr::WeakPtr(SceneElement *s,
				  QSharedPointer<SceneElementData> const &d):
  s(s), d(d) {
}

SceneElement::WeakPtr::WeakPtr(): s(0) {
}

SceneElement *SceneElement::WeakPtr::data() const {
  return d.isNull() ? 0 : s;
}

void SceneElement::WeakPtr::clear() {
  d.clear();
}

SceneElement::WeakPtr::operator bool() const {
  return !d.isNull();
}


SceneElement::WeakPtr SceneElement::weakref() {
  return SceneElement::WeakPtr(this, d);
}

void SceneElement::paint(QPainter *painter,
			 const QStyleOptionGraphicsItem *,
			 QWidget *) {
  if (isSelected()) {
    painter->setBrush(QBrush(Style::selectionBackgroundColor()));
    painter->setPen(QPen(Qt::NoPen));
    painter->setCompositionMode(QPainter::CompositionMode_Darken);
    painter->drawRoundedRect(boundingRect(),
			     Style::selectionRectRadius(),
			     Style::selectionRectRadius());
  }
  QGraphicsColorizeEffect *ef
    = dynamic_cast<QGraphicsColorizeEffect *>(d->element->graphicsEffect());
  if (ef) 
    ef->setColor(isSelected() ? Style::selectedElementHoverColor()
		 : Style::hoverColor());
}

QRectF SceneElement::boundingRect() const {
  return d->element->mapRectToParent(d->element->boundingRect()
				     .adjusted(-2, -2, 2, 2));
}

QString SceneElement::symbol() const {
  return d->sym;
}
