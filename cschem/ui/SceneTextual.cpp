// SceneTextual.cpp

#include "SceneTextual.h"
#include "circuit/Textual.h"
#include "Style.h"
#include "Scene.h"
#include "circuit/PartNumbering.h"

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
    moving = false;
  }
  void rebuild();
  void rebuildText();
  void rebuildPos();
  void recolor();
public:
  SceneTextual *st;
  Scene *scene;
  Textual txt;
  bool hovered;
  bool selected;
  bool mypress;
  bool moving;
  QPoint dp; // for temporary moves
  QPointF sp_press;
};

void STData::recolor() {
  if (hovered && !st->hasFocus()) {
    if (selected) 
      st->setDefaultTextColor(Style::selectedElementHoverColor());
    else
      st->setDefaultTextColor(Style::hoverColor());
  } else {
    st->setDefaultTextColor(Style::textColor());
  }
}


void STData::rebuild() {
  rebuildText();
  rebuildPos();
}

static QString subscript(QString ifx) {
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
  ifx.replace(".", "܂");
  return ifx;
}

static QString dehtml(QString ifx) {
  ifx.replace("₀", "0");
  ifx.replace("₁", "1");
  ifx.replace("₂", "2");
  ifx.replace("₃", "3");
  ifx.replace("₄", "4");
  ifx.replace("₅", "5");
  ifx.replace("₆", "6");
  ifx.replace("₇", "7");
  ifx.replace("₈", "8");
  ifx.replace("₉", "9");
  ifx.replace("܂", ".");
  ifx.replace("&lt;", "<");
  ifx.replace("&gt;", ">");
  ifx.replace("&amp;", "&");
  ifx.replace("&#10;", "\n");
  ifx.replace("−", "-");
  return ifx;
}  

static QString lineToHtml(QString line, QSet<QString> const &allnames) {
  /* Make sure this logic is copied in SvgExporter::writeTextualLine  */
  QRegularExpression minus("(^|(?<=\\s))-($|(?=[\\s.0-9]))");
  line.replace(minus, "−");
  QStringList bits;
  QString bit;
  bool inword = false;
  int len = line.length();
  for (int pos=0; pos<len; pos++) {
    QChar c = line[pos];
    if (bit=="") {
      bit = c;
      inword = c.isLetterOrNumber();
    } else if ((inword==c.isLetterOrNumber())
               || (inword && c==QChar('.')
                   && pos<len-1 && line[pos+1].isNumber())) {
        bit += c;
    } else {
      bits += bit;
      bit = c;
      inword = c.isLetterOrNumber();      
    }
  }
  if (bit != "")
    bits += bit;

  QRegularExpression presym("[*/+−=]");
  QRegularExpression postsym("[*/+−=_^]");
  QList<bool> mathcontext;
  for (int k=0; k<bits.size(); k++)  
    mathcontext << (bits[k].size()==1 && bits[k][0].isLetter()
                    && ((k>0 && bits[k-1].contains(presym))
                        || (k+1<bits.size() && bits[k+1].contains(postsym))));  
  
  QString html;
  bool insup = false;
  bool insub = false;
  for (int k=0; k<bits.size(); k++) {
    QString bit = bits[k];
    if (mathcontext[k]) {
      html += "<i>" + bit + "</i>";
      continue;
    }
    bit.replace("&", "&amp;");
    bit.replace("<", "&lt;");
    bit.replace(">", "&gt;");
    if (allnames.contains(bit)) {
      QString pfx = PartNumbering::prefix(bit);
      QString sfx = bit.mid(pfx.size());
      html += "<i>" + pfx + "</i>" + "<sub>" + sfx + "</sub>";
    } else if ((bit.startsWith("V") || bit.startsWith("I"))
               && allnames.contains(bit.mid(1))) {
      QString wht = bit.left(1);
      QString name = bit.mid(1);
      QString pfx = PartNumbering::prefix(name);
      QString sfx = name.mid(pfx.size());
      html += "<i>" + wht + "</i>"
        + "<sub>"
        + "<i>" + pfx + "</i>"
        + subscript(sfx)
        + "</sub>";
    } else if (bit=="^") {
      if (insub) {
        html += "</sub>";
        insub = false;
      }
      if (!insup) {
        html += "<sup>";
        insup = true;
      }
      html += "<span style=\"font-size: 2pt; color: white;\">" + bit + "</span>";

    } else if (bit=="_") {
      if (insup) {
        html += "</sup>";
        insup = false;
      }
      if (!insub) {
        html += "<sub>";
        insub = true;
      }
      html += "<span style=\"font-size: 2pt; color: white;\">" + bit + "</span>";
    } else {
      bit.replace("  ", " &nbsp;");
      html += bit;
      if (insup) {
        html += "</sup>";
        insup = false;
      }
      if (insub) {
        html += "</sub>";
        insub = false;
      }
    }
  }
  if (insup) {
    html += "</sup>";
    insup = false;
  }
  if (insub) {
    html += "</sub>";
    insub = false;
  }
  if (html.endsWith(" "))
    html = html.left(html.length() - 1) + "&nbsp;";
  //qDebug() << "hmtl" << html;
  return html;
}

void STData::rebuildText() {
  QSet<QString> allnames;
  if (scene)
    allnames = scene->circuit().allNames();
  if (txt.text.startsWith('"') && txt.text.size()>=2 && txt.text.endsWith('"')) 
    txt.text = "“" + txt.text.mid(1, txt.text.size()-2) + "”";
  txt.text.replace("<=", "≤"); 
  txt.text.replace(">=", "≥");
  txt.text.replace("<<", "≪");
  txt.text.replace(">>", "≫");
  txt.text.replace("!=", "≠");
  txt.text.replace("+-", "±");
  txt.text.replace("~~", "≈");
  txt.text.replace("Ohm", "Ω");
  txt.text.replace("uF", "μF");
  txt.text.replace("uH", "μH");
  txt.text.replace("uA", "μA");
  txt.text.replace("uV", "μV");
  // cannot do the same for "us".
  QStringList lines = txt.text.split("\n");
  for (int n=0; n<lines.size(); n++)
    lines[n] = lineToHtml(lines[n], allnames);
  
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
  setFlag(ItemAcceptsInputMethod);
  setAcceptHoverEvents(true);
  d->rebuild();
  d->scene->addItem(this);
  connect(document(), &QTextDocument::contentsChanged,
	  [this]() {
	    QString t = dehtml(document()->toPlainText());
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
  if (d->selected) {
    p->setBrush(QBrush(Style::selectionAnnotationBackgroundColor()));
    p->setPen(QPen(Qt::NoPen));
    p->setCompositionMode(QPainter::CompositionMode_Darken);
    p->drawRoundedRect(boundingRect(),
                       Style::selectionRectRadius(),
                       Style::selectionRectRadius());
  }
    
}

void SceneTextual::temporaryTranslate(QPoint delta) {
  d->dp = delta;
  d->rebuildPos();
} 

void SceneTextual::setSelected(bool s) {
  if (d->selected==s)
    return;
  d->selected = s;
  d->recolor();
  update();
}

bool SceneTextual::isSelected() const {
  return d->selected;
}

void SceneTextual::hoverEnterEvent(QGraphicsSceneHoverEvent *) {
  d->hovered = true;
  d->recolor();
  update();
}

void SceneTextual::hoverLeaveEvent(QGraphicsSceneHoverEvent *) {
  d->hovered = false;
  d->recolor();
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
  if (e->button()==Qt::LeftButton) {
    d->mypress = true;
    d->sp_press = e->scenePos();
    temporaryTranslate(QPoint());
    QGraphicsTextItem::mousePressEvent(e);
    e->accept();
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
    if (!d->moving && dp.manhattanLength()>=3) {
      d->moving = true;
      clearFocus();
      d->recolor();
    }
    if (d->moving)
      temporaryTranslate(d->scene->library().downscale(dp));
  } else {
    QGraphicsTextItem::mouseMoveEvent(e);
  }
}


QPoint SceneTextual::textPosition() const {
  return d->txt.position;
}

void SceneTextual::mouseReleaseEvent(QGraphicsSceneMouseEvent *e) {
  if (d->mypress) {
    d->mypress = false;
    if (d->moving)
      d->scene->repositionTextual(id(), d->txt.position + d->dp);
    d->moving = false;
    temporaryTranslate(QPoint());
  } else {
    QGraphicsTextItem::mouseReleaseEvent(e);
  }
}


void SceneTextual::focusInEvent(QFocusEvent *e) {
  QGraphicsTextItem::focusInEvent(e);
  d->recolor();
  d->scene->clearSelection();
}

void SceneTextual::focusOutEvent(QFocusEvent *e) {
  QGraphicsTextItem::focusOutEvent(e);
  d->recolor();
  if (d->txt.text.isEmpty())
    d->scene->dropTextual(id());
}
