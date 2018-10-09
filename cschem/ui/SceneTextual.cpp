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
#include <QRegularExpression>
#include <QTextCursor>

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

static QString rebuildLine(QString str) {
  QStringList bits = str.split(" ", QString::KeepEmptyParts);

  QRegularExpression vari("^([A-Z]+)([.0-9]*[0-9])([.,?!:;]?)$");
  QRegularExpression oper("^(=|\\+|\\-|\\<|\\>|≤|≥|≠|±|≈)$");
  QRegularExpression voltcurrent("^(V|I)([A-Za-z0-9]*)(.[0-9]+)?([.,?!:;]?)$");
  QRegularExpression name("^([A-Za-z])");
  QRegularExpression periodnum("\\.([0-9])");
  QRegularExpression ending("[.,?!:;]$");
  QRegularExpression singleletter("^[B-HJ-UW-Z][.,?!:;]?$");

  auto isMath = [=](int n) {
    if (n>=0 && n<bits.size()) {
      QString bit = bits[n];
      if (oper.match(bit).hasMatch())
	return true;
      else if (vari.match(bit).hasMatch())
	return true;
    }
    return false;
  };

  auto impliedMath = [=](int n) {
    if (n>=0 && n<bits.size()) {
      if (isMath(n) || singleletter.match(bits[n]).hasMatch())
	return true;
      if ((!isMath(n-1) || (n>0 && ending.match(bits[n-1]).hasMatch()))
	   && !isMath(n+1))
	return false;
      QString bit = bits[n];
      if (voltcurrent.match(bit).hasMatch())
	return true;
    }
  };

  auto markup = [=](QString str) {
    auto mtch = voltcurrent.match(str);
    if (mtch.hasMatch()) {
      QString pfx = mtch.captured(1);
      QString ifx = mtch.captured(2) + mtch.captured(3);
      QString sfx = mtch.captured(4);
      if (ifx>="A") {
	ifx.replace("0", "₀");
	ifx.replace("1", "₁");
	ifx.replace("2", "₂");
	ifx.replace("3", "₃");
	ifx.replace("4", "₄");
	ifx.replace("5", "₅");
	ifx.replace("6", "₆");
	ifx.replace("7", "₇");
	ifx.replace("8", "₈");
	ifx.replace("9", "₉");
	ifx.replace(periodnum, "܂" 
		    "\\1");
      }
      return "<i>" + pfx + "</i><sub>" + ifx + "</sub>" + sfx;
    } else {
      auto mtch = vari.match(str);
      if (mtch.hasMatch())
	return "<i>" + mtch.captured(1) + "</i><sub>"
	  + mtch.captured(2) + "</sub>" + mtch.captured(3);
      else if (name.match(str).hasMatch())
	return "<i>" + str + "</i>";
    }
    return str;
  };
    
  for (int n=0; n<bits.size(); n++)
    if (impliedMath(n))
      bits[n] = markup(bits[n]);

  return bits.join("&nbsp;");
}
  

void STData::rebuildText() {
  txt.text.replace("<=", "≤");
  txt.text.replace(">=", "≥");
  txt.text.replace("!=", "≠");
  txt.text.replace("+-", "±");
  txt.text.replace("~~", "≈");
  txt.text.replace("Ohm", "Ω");
  QStringList lines = txt.text.split("\n");
  for (int n=0; n<lines.size(); n++)
    lines[n] = rebuildLine(lines[n]);
  
  QTextCursor c(st->textCursor());
  int pos = c.position();
  st->document()->setHtml(lines.join("<br>"));
  c.setPosition(pos);
  st->setTextCursor(c);
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
  setFont(Style::annotationFont());
  setTextInteractionFlags(Qt::TextEditorInteraction);
  setFlags(ItemIsFocusable);
  setAcceptHoverEvents(true);
  d->rebuild();
  d->scene->addItem(this);
  connect(document(), &QTextDocument::contentsChanged,
	  [this]() {
	    QString t = document()->toPlainText();
	    if (t==d->txt.text)
	      return;
	    d->txt.text = t;
	    d->rebuildText();
	    d->scene->storeTextualText(id(), d->txt.text);
	  });
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
