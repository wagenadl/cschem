// SceneElement.cpp

#include "SceneElement.h"
#include "SceneElementData.h"
#include "ui/SvgItem.h"
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
  Element elt(scene->circuit().elements[id]);
  elt.nameVisible = false;
  scene->modifyElementAnnotations(elt);
}

void SceneElementData::removeValue() {
  Element elt(scene->circuit().elements[id]);
  elt.valueVisible = false;
  scene->modifyElementAnnotations(elt);
}

void SceneElementData::moveValue(QPointF delta) {
  if (delta.isNull())
    return;
  Element elt(scene->circuit().elements[id]);
  elt.valuePosition += delta.toPoint();
  scene->modifyElementAnnotations(elt);
}

void SceneElementData::moveName(QPointF delta) {
  if (delta.isNull())
    return;
  Element elt(scene->circuit().elements[id]);
  elt.namePosition += delta.toPoint();
  scene->modifyElementAnnotations(elt);
}

void SceneElementData::nameTextToWidget() {
  // Copy name from circuit to widget
  // Called when escape pressed
  Circuit const &circ = scene->circuit(); 
  Element const &elt(circ.elements[id]);
  QString txt = elt.name;
  if (txt.isEmpty()) {
    name->setHtml("<i>Ref</i>");
    name->setDefaultTextColor(Style::faintColor());
  } else {
    name->setHtml(PartNumbering::nameToHtml(txt));
    name->setDefaultTextColor(Style::textColor());
  }
  if (name->hasFocus())
    name->clearFocus();
}

void SceneElementData::nameTextToCircuit() {
  // Copy name from widget to circuit
  // Called when return pressed
  Element elt(scene->circuit().elements[id]);
  QString txt = name->toPlainText();
  if (txt == "Ref")
    txt = "";
  if (txt.isEmpty())
    txt = scene->circuit().autoName(elt.symbol());
  if (txt != elt.name) { // prevent recursion
    elt.name = txt;
    scene->modifyElementAnnotations(elt);
  }
}

void SceneElementData::valueTextToWidget() {
  // Copy value from circuit to widget
  QString txt = scene->circuit().elements[id].value;
  if (txt.isEmpty()) {
    value->setHtml("<i>P/V</i>");
    value->setDefaultTextColor(Style::faintColor());
  } else {
    value->setPlainText(txt);
    value->setDefaultTextColor(Style::textColor());
  }
  if (value->hasFocus())
    value->clearFocus();
}

void SceneElementData::valueTextToCircuit() {
  // Copy value from widget to circuit
  Element elt(scene->circuit().elements[id]);
  QString txt = value->toPlainText();
  if (txt == "P/V")
    txt = "";
  txt = PartNumbering::prettyValue(txt, elt.name);
  if (txt != elt.value) { // prevent recursion
    elt.value = txt;
    scene->modifyElementAnnotations(elt);
  }
}

SceneElement::SceneElement(class Scene *parent, Element const &elt):
  d(new SceneElementData) {
  //
  d->scene = parent;
  d->id = elt.id;
  d->sym = elt.symbol();

  SymbolLibrary const &lib = d->scene->library();
  Symbol const &symbol = lib.symbol(d->sym);
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
}

SceneElement::~SceneElement() {
}

void SceneElement::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *) {
  Element elt = d->scene->circuit().elements[d->id];
  if (elt.type == Element::Type::Component
      || elt.type == Element::Type::Port) {
    elt.nameVisible = true;
    if (elt.type == Element::Type::Component)
      elt.valueVisible = true;
    d->scene->modifyElementAnnotations(elt);
  }
}

void SceneElement::mousePressEvent(QGraphicsSceneMouseEvent *e) {
  qDebug() << "element press" << e->pos() << boundingRect();
  d->dragmoved = false;
  Circuit const &circ = d->scene->circuit();
  SymbolLibrary const &lib = d->scene->library();
  d->delta0 = circ.elements[d->id].position - lib.downscale(e->scenePos());
  e->accept();
}

void SceneElement::mouseMoveEvent(QGraphicsSceneMouseEvent *e) {
  auto const &lib = d->scene->library();
  auto const &circ = d->scene->circuit();
  QPoint newpos = lib.downscale(e->scenePos()) + d->delta0;
  QPoint oldpos = circ.elements[d->id].position;
  
  if (d->dragmoved || newpos != oldpos) {
    d->scene->tentativelyMoveSelection(newpos - oldpos, !d->dragmoved,
				       e->modifiers() & Qt::ControlModifier);
    d->dragmoved = true;
  }
  e->accept();
}

void SceneElement::mouseReleaseEvent(QGraphicsSceneMouseEvent *e) {
  auto const &lib = d->scene->library();
  auto const &circ = d->scene->circuit();
  QPoint newpos = lib.downscale(e->scenePos()) + d->delta0;
  QPoint oldpos = circ.elements[d->id].position;
  if (d->dragmoved || newpos != oldpos) {
    d->scene->moveSelection(newpos - oldpos,
			    e->modifiers() & Qt::ControlModifier);
  }
  e->accept();
}

void SceneElement::temporaryTranslate(QPoint delta) {
  Circuit const &circ = d->scene->circuit();
  Element elt(circ.elements[d->id]);
  SymbolLibrary const &lib = d->scene->library();

  setPos(lib.upscale(elt.position + delta));
}

void SceneElement::rebuild() {
  // Reconstruct element
  Circuit const &circ = d->scene->circuit();
  Element elt(circ.elements[d->id]);
  SymbolLibrary const &lib = d->scene->library();
  Symbol const &symbol = lib.symbol(elt.symbol());

  prepareGeometryChange();

  d->element->setRenderer(symbol.renderer());
  
  QPointF orig = symbol.bbOrigin();
  QTransform xf;
  xf.translate(orig.x(), orig.y());
  xf.rotate(elt.rotation*-90);
  if (elt.flipped)
    xf.scale(-1, 1);
  xf.translate(-orig.x(), -orig.y());
  d->element->setTransform(xf);
  d->element->setPos(symbol.shiftedBBox().topLeft());

  setPos(lib.scale() * elt.position);

  // Reconstruct name
  if (elt.nameVisible) {
    if (d->name) {
      d->name->show();
    } else {
      d->name = new SceneAnnotation(d->scene->library().scale(), this);
      QObject::connect(d->name, SIGNAL(returnPressed()),
		       d.data(), SLOT(nameTextToCircuit()));
      QObject::connect(d->name, SIGNAL(escapePressed()),
		       d.data(), SLOT(nameTextToWidget()));
      QObject::connect(d->name, SIGNAL(moved(QPointF)),
		       d.data(), SLOT(moveName(QPointF)));
      QObject::connect(d->name, SIGNAL(removalRequested()),
		       d.data(), SLOT(removeName()));
      QObject::connect(d->name, SIGNAL(hovering(bool)),
                       d.data(), SLOT(nameHovering(bool)));
    }

    QPoint p = elt.namePosition;
    if (p.isNull()) {
      Geometry geom(circ, lib);
      QRectF abb = geom.defaultAnnotationSvgBoundingRect(elt, "name");
      p = abb.center().toPoint();
      elt.namePosition = p;
      d->scene->modifyElementAnnotations(elt);
    }
    d->nameTextToWidget();
    d->name->setCenter(p);
    if (elt.name.isEmpty()) {
      d->nameTextToCircuit(); // automated magic
      elt = circ.elements[d->id];
    }
  } else if (d->name) {
    d->name->hide();
  }
  
  // Reconstruct value
  if (elt.valueVisible) {
    if (d->value) {
      d->value->show();
    } else {
      d->value = new SceneAnnotation(d->scene->library().scale(), this);
      QObject::connect(d->value, SIGNAL(returnPressed()),
		       d.data(), SLOT(valueTextToCircuit()));
      QObject::connect(d->value, SIGNAL(escapePressed()),
		       d.data(), SLOT(valueTextToWidget()));
      QObject::connect(d->value, SIGNAL(moved(QPointF)),
		       d.data(), SLOT(moveValue(QPointF)));
      QObject::connect(d->value, SIGNAL(removalRequested()),
		       d.data(), SLOT(removeValue()));
      QObject::connect(d->name, SIGNAL(hovering(bool)),
                       d.data(), SLOT(valueHovering(bool)));
    }
    QPoint p = elt.valuePosition;
    if (p.isNull()) {
      Geometry geom(circ, lib);
      QRectF abb = geom.defaultAnnotationSvgBoundingRect(elt, "value");
      p = abb.center().toPoint();
      elt.valuePosition = p;
      d->scene->modifyElementAnnotations(elt);
    }
    d->valueTextToWidget();
    d->value->setCenter(p);
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
  if (d->sym=="junction") {
    return d->element->mapRectToParent(d->element->boundingRect()
                                       .adjusted(-6, -6, 6, 6));
  } else {
    return d->element->mapRectToParent(d->element->boundingRect()
                                       .adjusted(-2, -2, 2, 2));
  }
}

QString SceneElement::symbol() const {
  return d->sym;
}

void SceneElement::setSelected(bool s) {
  d->selected = s;
  d->scene->perhapsEmitSelectionChange();
  update();
}

bool SceneElement::isSelected() const {
  return d->selected;
}
