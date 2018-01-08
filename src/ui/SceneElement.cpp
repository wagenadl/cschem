// SceneElement.cpp

#include "SceneElement.h"
#include <QGraphicsSvgItem>
#include <QGraphicsEllipseItem>
#include <QGraphicsItemGroup>
#include "file/Element.h"
#include "Scene.h"
#include <QDebug>
#include <QGraphicsColorizeEffect>
#include <QGraphicsSceneMouseEvent>
#include "Style.h"
#include <QPainter>

class SceneElementData {
public:
  SceneElementData() {
    scene = 0;
    id = 0;
    element = 0;
    name = 0;
    value = 0;
    dragmoved = false;
    hover = false;
  }
public:
  void markHover() {
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
public:
  Scene *scene;
  int id;
  QGraphicsSvgItem *element;
  QGraphicsTextItem *name;
  QGraphicsTextItem *value;
public:
  bool dragmoved;
  bool hover;
};

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

  d->element = new QGraphicsSvgItem;
  if (r)
    d->element->setSharedRenderer(r);
  else
    qDebug() << "Cannot construct renderer for symbol" << sym;    
  addToGroup(d->element);

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

void SceneElement::mousePressEvent(QGraphicsSceneMouseEvent *e) {
  d->dragmoved = false;
  qDebug() << "Element Mouse press" << e->pos() << pos();
  QGraphicsItemGroup::mousePressEvent(e);
}

void SceneElement::mouseMoveEvent(QGraphicsSceneMouseEvent *e) {
  QPointF newpos = pos();
  QPointF oldpos = d->scene->library()->scale()
    * d->scene->circuit().elements()[d->id].position();
  qDebug() << "Mouse move" << newpos << oldpos;
  QGraphicsItemGroup::mouseMoveEvent(e);
  
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
  QGraphicsItemGroup::mouseReleaseEvent(e);
  if (d->dragmoved || newpos != oldpos) {
    qDebug() << "Position changed";
    d->scene->moveSelection(newpos - oldpos);
  }
}

void SceneElement::rebuild() {
  // Reconstruct element
  removeFromGroup(d->element);
  setPos(QPointF(0,0));
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
  addToGroup(d->element);
  setPos(lib->scale() * circ.element(d->id).position());

  // Reconstruct name
  Element const &elt(circ.element(d->id));
  if (elt.isNameVisible()) {
    qDebug() << "name visible on" << elt.report();
    if (d->name) {
      d->name->show();
    } else {
      d->name = new QGraphicsTextItem;
      d->name->setFont(Style::nameFont());
      d->name->setTextInteractionFlags(Qt::TextEditorInteraction);
      d->name->setFlags(ItemIsFocusable);
      addToGroup(d->name);
    }
    QPoint p = elt.namePos();
    if (p.isNull())
      p = QPoint(10, 10); // hmmm.
    d->name->setPos(p);
    QString name = elt.name();
    if (name.mid(1).toInt()>0) 
      // letter+number
      d->name->setHtml("<i>" + name.left(1) + "</i>"
                       + "<sub>" + name.mid(1) + "</sub>");
    else
      d->name->setHtml("<i>" + name + "</i>");
  } else if (d->name) {
    d->name->hide();
  }

  
  // Reconstruct value
  if (elt.isValueVisible()) {
    if (d->value) {
      d->value->show();
    } else {
      d->value = new QGraphicsTextItem;
      d->value->setFont(Style::valueFont());
      d->value->setTextInteractionFlags(Qt::TextEditorInteraction);
      d->value->setFlags(ItemIsFocusable);
      addToGroup(d->value);
    }
    QPoint p = elt.valuePos();
    if (p.isNull())
      p = QPoint(30, 10); // hmmm.
    d->value->setPos(p);
    QString value = elt.value();
    d->value->setPlainText(value);
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
    painter->drawRoundedRect(d->element->mapRectToParent(d->element->boundingRect().adjusted(2, 2, -2, -2)),
			     Style::selectionRectRadius(),
			     Style::selectionRectRadius());
  }
  QGraphicsColorizeEffect *ef
    = dynamic_cast<QGraphicsColorizeEffect *>(d->element->graphicsEffect());
  if (ef) 
    ef->setColor(isSelected() ? Style::selectedElementHoverColor()
		 : Style::elementHoverColor());
}
