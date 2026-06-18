// ElementView.cpp

#include "ElementView.h"
#include "Editor.h"
#include "data/Object.h"

#include <QPainter>
#include <QTextDocument>
#include <QMouseEvent>

ElementView::ElementView(QWidget *parent): ComponentView(parent) {
  cvmap()[id()] = this;
  ed = 0;
}

ElementView::~ElementView() {
  cvmap().remove(id()); 
}

QString ElementView::refText() const {
  return ref;
}

QString ElementView::pvText() const {
  return pv;
}

void ElementView::setRefText(QString s) {
  ref = s;
  relabel();
}

void ElementView::setPVText(QString s) {
  pv = s;
  relabel();
}

void ElementView::relabel() {
  reflbl = "<body><b>";
  bool isNum = ref.mid(1).toDouble()>0;
  int midStart = isNum ? 1 : ref.indexOf("_");
  int leftLen = isNum ? midStart : midStart - 1;
  if (midStart>0)
    reflbl += "<i>" + ref.left(leftLen) + "</i>"
      + "<sub>" + ref.mid(midStart) + "</sub>";
  else
    reflbl += ref;
  reflbl += "</b></body>";
  pvlbl = "<body>" + pv + "</body>";
  update();
}

QMap<int, ElementView *> &ElementView::cvmap() {
  static QMap<int, ElementView *> m;
  return m;
}

ElementView *ElementView::instance(int id) {
  if (cvmap().contains(id))
    return cvmap()[id];
  else
    return 0;
}

void ElementView::paintEvent(QPaintEvent *e) {
  ComponentView::paintEvent(e);
  QPainter p(this);
  QTextDocument doc;
  doc.setDefaultStyleSheet("body { color: #ffffff; }");

  doc.setHtml(reflbl);
  doc.drawContents(&p);

  doc.setHtml(pvlbl);
  p.translate(width() - doc.size().width(), height() - doc.size().height());
  doc.drawContents(&p);
}

void ElementView::linkEditor(Editor *ed1) {
  ed = ed1;
}

void ElementView::mousePressEvent(QMouseEvent *e) {
  if ((e->button()==Qt::LeftButton && e->modifiers() & Qt::ControlModifier)
      || e->button()==Qt::MiddleButton) {
    pasteOutlineFromEditor();
    e->accept();
  } else {
    ComponentView::mousePressEvent(e);
  }
}

void ElementView::pasteOutlineFromEditor() {
  setGroup(Group());
  if (!ed) {
    return;
  }

  QSet<int> sel(ed->selectedObjects());
  Group const &cg(ed->currentGroup());
  int gid = -1;
  for (int id: sel) {
    if (cg.object(id).isGroup()) {
      if (gid>0) 
        return;
      else 
        gid = id;
    }
  }
  if (gid>0)
    setGroup(cg.object(gid).asGroup());
}

