// SceneElement.cpp

#include "SceneElement.h"
#include "SceneElementData.h"
#include <QGraphicsSvgItem>
#include <QGraphicsEllipseItem>
#include "file/Element.h"
#include "Scene.h"
#include <QDebug>
#include <QGraphicsColorizeEffect>
#include <QGraphicsSceneMouseEvent>
#include "SceneAnnotation.h"
#include "Style.h"
#include <QPainter>
#include "svg/PartNumbering.h"

SceneElementData::~SceneElementData() {
}

void SceneElementData::markHover() {
  if (hover) {
    auto *ef = new QGraphicsColorizeEffect;
    ef->setColor(element->parentItem()->isSelected()
		 ? Style::selectedElementHoverColor()
		 : Style::elementHoverColor());
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
    if (txt.mid(1).toInt()>0) 
      // letter+number
      name->setHtml("<i>" + txt.left(1) + "</i>"
		    + "<sub>" + txt.mid(1) + "</sub>");
    else
      name->setHtml("<i>" + txt + "</i>");
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
  qDebug() << "getnametext" << txt;
  if (txt.isEmpty())
    txt = PartNumbering::abbreviation(elt.symbol());
  elt.setName(txt);
  circ.insert(elt);
  qDebug() << "txt" << txt;
  if (txt.size()==1 && txt[0].isLetter()) {
    // inserting it twice is required, otherwise autoname won't work
    txt = circ.autoName(txt);
    elt.setName(txt);
    circ.insert(elt);
  }
  qDebug() << "==>" << scene->circuit().element(id).name();
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
  if (elt.name().mid(1).toInt()>0) {
    if (elt.name().startsWith("R") && txt.endsWith("."))
      txt = txt.left(txt.size() - 1) + tr("Ω");
    else if (elt.name().startsWith("C") || elt.name().startsWith("L"))
      txt = txt.replace("u", tr("μ"));
  }
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
  QString sym = elt.symbol();

  PartLibrary const *lib = d->scene->library();
  Part const &part = lib->part(sym);
  if (!part.isValid())
    qDebug() << "Cannot find svg for symbol" << sym;

  QSvgRenderer *r = lib->renderer(sym).data();

  d->element = new QGraphicsSvgItem(this);
  if (r)
    d->element->setSharedRenderer(r);
  else
    qDebug() << "Cannot construct renderer for symbol" << sym;    
  
  parent->addItem(this);
  rebuild();

  if (sym == "junction")
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
  elt.setNameVisible(true);
  elt.setValueVisible(true);
  d->scene->circuit().insert(elt);
  rebuild();
}

void SceneElement::mousePressEvent(QGraphicsSceneMouseEvent *e) {
  d->dragmoved = false;
  qDebug() << "Element Mouse press" << e->pos() << pos();
  QGraphicsItem::mousePressEvent(e);
}

void SceneElement::mouseMoveEvent(QGraphicsSceneMouseEvent *e) {
  QPointF newpos = pos();
  QPointF oldpos = d->scene->library()->scale()
    * d->scene->circuit().elements()[d->id].position();
  qDebug() << "Mouse move" << newpos << oldpos;
  QGraphicsItem::mouseMoveEvent(e);
  
  if (d->dragmoved || newpos != oldpos) {
    qDebug() << "Position changed";
    d->scene->tentativelyMoveSelection(newpos - oldpos);
    d->dragmoved = true;
  }
}

void SceneElement::mouseReleaseEvent(QGraphicsSceneMouseEvent *e) {
  auto const &lib = d->scene->library();
  auto const &circ = d->scene->circuit();
  QPointF newpos = pos();
  QPointF oldpos = lib->scale() * circ.elements()[d->id].position();
  qDebug() << "Mouse release" << newpos << oldpos;
  QGraphicsItem::mouseReleaseEvent(e);
  if (d->dragmoved || newpos != oldpos) {
    qDebug() << "Position changed";
    d->scene->moveSelection(newpos - oldpos);
  }
}

void SceneElement::rebuild() {
  // Reconstruct element
  prepareGeometryChange();
  
  PartLibrary const *lib = d->scene->library();
  Circuit const &circ = d->scene->circuit();
  Part const &part = lib->part(circ.element(d->id).symbol());
  QPointF orig = part.bbOrigin();
  QTransform xf;
  xf.translate(orig.x(), orig.y());
  xf.rotate(circ.element(d->id).rotation()*-90);
  xf.translate(-orig.x(), -orig.y());
  d->element->setTransform(xf);
  d->element->setPos(part.shiftedBBox().topLeft());

  setPos(lib->scale() * circ.element(d->id).position());

  // Reconstruct name
  Element elt(circ.element(d->id));
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
    }
    QPoint p = elt.namePos();
    if (p.isNull()) {
      p = QPoint(0, 3 * d->scene->library()->scale()); // hmmm.
      elt.setNamePos(p);
      d->scene->circuit().insert(elt);
    }
    d->name->setPos(p);
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
    }
    QPoint p = elt.valuePos();
    if (p.isNull()) {
      p = QPoint(0, 6 * d->scene->library()->scale()); // hmmm.
      elt.setValuePos(p);
      d->scene->circuit().insert(elt);
    }
    d->value->setPos(p);
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
    painter->setBrush(QBrush(Style::selectionColor()));
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
		 : Style::elementHoverColor());
}

QRectF SceneElement::boundingRect() const {
  return d->element->mapRectToParent(d->element->boundingRect()
				     .adjusted(-2, -2, 2, 2));
}
