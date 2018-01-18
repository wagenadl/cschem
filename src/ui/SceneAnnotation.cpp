// SceneAnnotation.cpp

#include "SceneAnnotation.h"
#include <QKeyEvent>
#include <QGraphicsSceneMouseEvent>
#include "Style.h"
#include <QTextDocument>
#include <QTextBlock>
#include <QTextLayout>
#include <QDebug>
#include <QGraphicsColorizeEffect>

class SAData {
public:
  SAData(double ms):
    movestep(ms),
    pressing(false),
    moving(false) {
  }
public:
  double movestep;
  QPointF sp_press; // scene position of mouse button press
  QPointF p_orig; // my position before moving
  bool pressing;
  bool moving;
  QString origtext;
};

SceneAnnotation::SceneAnnotation(double movestep, QGraphicsItem *parent):
  QGraphicsTextItem(parent), d(new SAData(movestep)) {
  setFont(Style::annotationFont());
  setTextInteractionFlags(Qt::TextEditorInteraction);
  setFlags(ItemIsFocusable);
  setAcceptHoverEvents(true);
}

SceneAnnotation::~SceneAnnotation() {
  delete d;
}

void SceneAnnotation::focusOutEvent(QFocusEvent *e) {
  QGraphicsTextItem::focusOutEvent(e);
  emit returnPressed(); // hmmm..
}

void SceneAnnotation::focusInEvent(QFocusEvent *e) {
  QGraphicsTextItem::focusInEvent(e);
  setGraphicsEffect(0);
}
 
void SceneAnnotation::keyPressEvent(QKeyEvent *e) {
  switch (e->key()) {
  case Qt::Key_Return: case Qt::Key_Enter:
    emit returnPressed();
    e->accept();
    break;
  case Qt::Key_Escape:
    emit escapePressed();
    e->accept();
    break;
  default:
    QGraphicsTextItem::keyPressEvent(e);
    break;
  }
}

void SceneAnnotation::mousePressEvent(QGraphicsSceneMouseEvent *e) {
  if (defaultTextColor() == Style::faintColor()) {
    d->origtext = toHtml();
    setPlainText("");
    setDefaultTextColor(Style::textColor());
  } else {
    d->origtext = "";
  }
  if (e->button() == Qt::LeftButton && d->movestep > 0) {
    d->sp_press = e->scenePos();
    d->pressing = true;
    d->p_orig = pos();
    e->accept();
  } else {
    QGraphicsTextItem::mousePressEvent(e);
  }
}

void SceneAnnotation::mouseMoveEvent(QGraphicsSceneMouseEvent *e) {
  if (d->pressing) {
    QPointF delta = e->scenePos() - d->sp_press;
    if (!d->moving && delta.manhattanLength() >= 3) {
      d->moving = true;
      if (d->origtext != "") {
        setHtml(d->origtext);
        setDefaultTextColor(Style::faintColor());
      }
      clearFocus();
    }      
    if (d->moving) {
      QPoint d0 = (delta / d->movestep).toPoint();
      setPos(d->p_orig + QPointF(d0) * d->movestep);
    }
  } else {
    QGraphicsTextItem::mouseMoveEvent(e);
  }      
}

void SceneAnnotation::mouseReleaseEvent(QGraphicsSceneMouseEvent *e) {
  if (d->pressing) {
    d->pressing = false;
    if (d->moving) {
      emit moved(pos() - d->p_orig);
      d->moving = false;
    }
  } else {
    QGraphicsTextItem::mouseReleaseEvent(e);
  }
}

void SceneAnnotation::backspace() {
  emit removalRequested();
}

void SceneAnnotation::setBaseline(QPointF p) {
  QTextLayout *lay = document()->firstBlock().layout();
  QPointF p0 = lay->position();
  QTextLine line = lay->lineAt(0);
  QPointF p1 = line.position();
  setPos(p - p0 - p1 - QPointF(0, line.ascent()));
}

void SceneAnnotation::hoverEnterEvent(QGraphicsSceneHoverEvent *) {
  auto *ef = new QGraphicsColorizeEffect;
  ef->setColor(Style::hoverColor());
  setGraphicsEffect(ef);
  emit hovering(true);
}
  
void SceneAnnotation::hoverLeaveEvent(QGraphicsSceneHoverEvent *) {
  setGraphicsEffect(0);
  emit hovering(false);
}
