// ElementView.cpp

#include "ElementView.h"
#include <QRegExp>
#include <QPainter>
#include <QTextDocument>

ElementView::ElementView(QWidget *parent): ComponentView(parent) {
  cvmap()[id()] = this;
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
