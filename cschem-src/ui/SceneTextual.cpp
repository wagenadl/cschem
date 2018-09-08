// SceneTextual.cpp

#include "SceneTextual.h"
#include "circuit/Textual.h"
#include "Style.h"
#include "Scene.h"

#include <QPainter>
#include <QTextDocument>
#include <QTextLayout>
#include <QTextBlock>
#include <QTextLine>
#include <QGraphicsSceneMouseEvent>

class STData {
public:
  STData(SceneTextual *st, Scene *scn, Textual const &t):
    st(st), scene(scn), txt(t) {
    hovered = false;
    selected = false;
    mypress = false;
  }
  void rebuild();
  void rebuildText();
  void rebuildPos();
public:
  SceneTextual *st;
  Scene *scene;
  Textual txt;
  bool hovered;
  bool selected;
  bool mypress;
  QPoint dp; // for temporary moves
  QPointF sp_press;
};

void STData::rebuild() {
  rebuildText();
  rebuildPos();
}

void STData::rebuildText() {
  st->document()->setPlainText(txt.text);
}

void STData::rebuildPos() {
  QTextLayout *lay = st->document()->firstBlock().layout();
  QPointF p0 = lay->position();
  QTextLine line = lay->lineAt(0);
  QPointF p1 = line.position();
  double ybase = p0.y() + p1.y() + line.ascent();
  double ycenter = ybase - Style::annotationFont().pixelSize() * .3;
  double xbase = p0.x() + p1.x();

  // When changing the math here, don't forget SvgExporter!
  st->setPos(scene->library().upscale(txt.position + dp)
	     - QPointF(xbase, ycenter));
}

SceneTextual::SceneTextual(class Scene *parent, class Textual const &txt):
  d(new STData(this, parent, txt)) {
  qDebug() << "SceneTextual" << txt;
  setFont(Style::annotationFont());
  setTextInteractionFlags(Qt::TextEditorInteraction);
  setFlags(ItemIsFocusable);
  setAcceptHoverEvents(true);
  d->rebuild();
  d->scene->addItem(this);
  connect(document(), &QTextDocument::contentsChanged,
	  [this]() { QString t = document()->toPlainText();
	    d->txt.text = t;
	    d->scene->storeTextualText(id(), t); });
}
  
SceneTextual::~SceneTextual() {
  delete d;
}

void SceneTextual::setTextual(Textual const &txt) {
  d->txt = txt;
  d->rebuild();
}

Scene *SceneTextual::scene() {
  return d->scene;
}

int SceneTextual::id() const {
  return d->txt.id;
}

void SceneTextual::paint(QPainter *p,
			 QStyleOptionGraphicsItem const *i,
			 QWidget *w) {
  // future vsn might draw s/th for hover or selected
  QGraphicsTextItem::paint(p, i, w);
}

void SceneTextual::temporaryTranslate(QPoint delta) {
  d->dp = delta;
  d->rebuildPos();
}

void SceneTextual::setSelected(bool s) {
  d->selected = s;
  update();
}

bool SceneTextual::isSelected() const {
  return d->selected;
}

void SceneTextual::hoverEnterEvent(QGraphicsSceneHoverEvent *) {
  d->hovered = true;
  update();
}

void SceneTextual::hoverLeaveEvent(QGraphicsSceneHoverEvent *) {
  d->hovered = false;
  update();
}

void SceneTextual::keyPressEvent(QKeyEvent *e) {
  switch (e->key()) {
  case Qt::Key_Escape:
    clearFocus();
    break;
  default:
    QGraphicsTextItem::keyPressEvent(e);
  }
}

void SceneTextual::mousePressEvent(QGraphicsSceneMouseEvent *e) {
  if (e->modifiers() & Qt::ControlModifier) {
    d->mypress = true;
    d->sp_press = e->scenePos();
    temporaryTranslate(QPoint());
    clearFocus();
  } else {
    d->mypress = false;
    QGraphicsTextItem::mousePressEvent(e);
  }
}

void SceneTextual::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *e) {
  QGraphicsTextItem::mouseDoubleClickEvent(e);
}

void SceneTextual::mouseMoveEvent(QGraphicsSceneMouseEvent *e) {
  if (d->mypress) {
    QPointF dp = e->scenePos() - d->sp_press;
    temporaryTranslate(d->scene->library().downscale(dp));
  } else {
    QGraphicsTextItem::mouseMoveEvent(e);
  }
}

void SceneTextual::mouseReleaseEvent(QGraphicsSceneMouseEvent *e) {
  if (d->mypress) {
    d->mypress = false;
    d->scene->repositionTextual(id(), d->txt.position + d->dp);
    temporaryTranslate(QPoint());
  } else {
    QGraphicsTextItem::mouseReleaseEvent(e);
  }
}

void SceneTextual::focusOutEvent(QFocusEvent *e) {
  QGraphicsTextItem::focusOutEvent(e);
  if (d->txt.text.isEmpty())
    d->scene->dropTextual(id());
}
