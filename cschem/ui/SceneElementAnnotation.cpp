// SceneElementAnnotation.cpp

#include "SceneElementAnnotation.h"
#include <QKeyEvent>
#include <QGraphicsSceneMouseEvent>
#include "Style.h"
#include <QTextDocument>
#include <QTextBlock>
#include <QTextLayout>
#include <QDebug>
#include <QGraphicsColorizeEffect>
#include <QPainter>

class SAData {
public:
  SAData(double ms):
    movestep(ms),
    pressing(false),
    moving(false),
    hovering(false),
    forcedhover(false),
    markedsel(false),
    faint(false) {
  }
public:
  double movestep;
  QPointF sp_press; // scene position of mouse button press
  QPointF p_orig; // my position before moving
  QPointF p_center; // officially requested center position
  bool pressing;
  bool moving;
  bool hovering; // real hovering
  bool forcedhover; // from parent
  bool markedsel;
  bool faint;
  QString placeholdertext;
public:
  void updateColor(SceneElementAnnotation *);
};

void SAData::updateColor(SceneElementAnnotation *sa) {
  if ((hovering || forcedhover) && !sa->hasFocus()) {
    if (markedsel) 
      sa->setDefaultTextColor(faint
                              ? Style::faintColor()
                              : Style::selectedElementHoverColor());
    else
      sa->setDefaultTextColor(faint
                              ? Style::faintHoverColor()
                              : Style::hoverColor());
  } else {
    sa->setDefaultTextColor(faint
                            ? Style::faintColor()
                            : Style::textColor());
  }
}

SceneElementAnnotation::SceneElementAnnotation(double movestep,
                                               QGraphicsItem *parent):
  QGraphicsTextItem(parent), d(new SAData(movestep)) {
  setFont(Style::annotationFont());
  setTextInteractionFlags(Qt::TextEditorInteraction);
  setFlags(ItemIsFocusable);
  setFlag(ItemAcceptsInputMethod);
  setAcceptHoverEvents(true);
  connect(document(), &QTextDocument::contentsChange,
	  this, &SceneElementAnnotation::updateCenter, Qt::QueuedConnection);
}

SceneElementAnnotation::~SceneElementAnnotation() {
  delete d;
}

void SceneElementAnnotation::focusOutEvent(QFocusEvent *e) {
  if (toPlainText()=="") {
    setFaint(true);
    setHtml(d->placeholdertext);
  } else {
    d->updateColor(this);
  }
  QGraphicsTextItem::focusOutEvent(e);
  QTextCursor tc = textCursor();
  if (tc.hasSelection()) {
    tc.clearSelection();
    setTextCursor(tc);
  }
  emit returnPressed(); // hmmm..
}

void SceneElementAnnotation::focusInEvent(QFocusEvent *e) {
  QGraphicsTextItem::focusInEvent(e);
  d->updateColor(this);
  emit focused();
}
 
void SceneElementAnnotation::keyPressEvent(QKeyEvent *e) {
  switch (e->key()) {
  case Qt::Key_Return: case Qt::Key_Enter:
    emit returnPressed();
    clearFocus();
    e->accept();
    break;
  case Qt::Key_Escape:
    emit escapePressed();
    clearFocus();
    e->accept();
    break;
  default:
    QGraphicsTextItem::keyPressEvent(e);
    break;
  }
}

void SceneElementAnnotation::mousePressEvent(QGraphicsSceneMouseEvent *e) {
  if (d->faint) {
    setPlainText("");
    setFaint(false);
  } 
  if (e->button() == Qt::LeftButton && d->movestep > 0) {
    d->sp_press = e->scenePos();
    d->pressing = true;
    d->p_orig = d->p_center;
    QGraphicsTextItem::mousePressEvent(e);
    e->accept();
  } else {
    QGraphicsTextItem::mousePressEvent(e);
  }
}

void SceneElementAnnotation::mouseMoveEvent(QGraphicsSceneMouseEvent *e) {
  if (d->pressing) {
    QPointF delta = e->scenePos() - d->sp_press;
    if (!d->moving && delta.manhattanLength() >= 3) {
      d->moving = true;
      if (d->faint) {
        setHtml(d->placeholdertext);
      }
      clearFocus();
      d->updateColor(this);
    }      
    if (d->moving) {
      QPoint d0 = (delta / d->movestep).toPoint();
      setCenter(d->p_orig + QPointF(d0) * d->movestep);
    }
  } else {
    QGraphicsTextItem::mouseMoveEvent(e);
  }      
}

void SceneElementAnnotation::mouseReleaseEvent(QGraphicsSceneMouseEvent *e) {
  if (d->pressing) {
    d->pressing = false;
    if (d->moving) {
      emit moved(d->p_center - d->p_orig);
      d->moving = false;
    }
  } else {
    QGraphicsTextItem::mouseReleaseEvent(e);
  }
}

void SceneElementAnnotation::backspace() {
  emit removalRequested();
}

void SceneElementAnnotation::setCenter(QPointF p) {
  d->p_center = p;
  updateCenter();
}

void SceneElementAnnotation::updateCenter() {
  QRectF me = boundingRect();
  double xcenter = me.width() / 2;

  QTextLayout *lay = document()->firstBlock().layout();
  QPointF p0 = lay->position();
  QTextLine line = lay->lineAt(0);
  QPointF p1 = line.position();
  double ybase = p0.y() + p1.y() + line.ascent();
  double ycenter = ybase - Style::annotationFont().pixelSize() * .3;
  // When changing the math here, don't forget SvgExporter!
  setPos(d->p_center - QPointF(xcenter, ycenter));
}

void SceneElementAnnotation::hoverEnterEvent(QGraphicsSceneHoverEvent *) {
  d->hovering = true;
  d->updateColor(this);
  emit hovering(true);
}
  
void SceneElementAnnotation::hoverLeaveEvent(QGraphicsSceneHoverEvent *) {
  d->hovering = false;
  d->updateColor(this);
  emit hovering(false);
}

void SceneElementAnnotation::forceHoverColor(bool x) {
  d->forcedhover = x;
  d->updateColor(this);
}

void SceneElementAnnotation::markSelected(bool x) {
  d->markedsel = x;
  d->updateColor(this);
  update();
}

void SceneElementAnnotation::setPos(QPointF const &p) {
  QGraphicsTextItem::setPos(p);
}

void SceneElementAnnotation::paint(QPainter *painter,
			 const QStyleOptionGraphicsItem *style,
			 QWidget *w) {
  QGraphicsTextItem::paint(painter, style, w);
  if (d->markedsel) {
    painter->setBrush(QBrush(Style::selectionAnnotationBackgroundColor()));
    painter->setPen(QPen(Qt::NoPen));
    painter->setCompositionMode(QPainter::CompositionMode_Darken);
    painter->drawRoundedRect(boundingRect(),
			     Style::selectionRectRadius(),
			     Style::selectionRectRadius());
  }
}

void SceneElementAnnotation::setFaint(bool x) {
  d->faint = x;
  if (x)
    setHtml(d->placeholdertext);
  d->updateColor(this);
}

void SceneElementAnnotation::setPlaceholderText(QString s) {
  d->placeholdertext = s;
  if (d->faint)
    setHtml(s);
}
