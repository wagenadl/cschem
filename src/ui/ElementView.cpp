// ElementView.cpp

#include "ElementView.h"
#include "ComponentView.h"
#include <QLabel>
#include <QVBoxLayout>
#include <QRegExp>

ElementView::ElementView(QWidget *parent): QWidget(parent) {
  l1 = new QLabel;
  l2 = 0;
  cv = new ComponentView;
  cvmap()[cv->id()] = this;

  auto *lay = new QVBoxLayout;
  lay->setMargin(0);
  lay->setSpacing(0);
  lay->addWidget(l1);
  lay->addWidget(cv);
  setLayout(lay);

  // auto p = l1->palette();
  // p.setColor(QPalette::WindowText, QColor(255, 255, 255));
  // p.setColor(QPalette::Text, QColor(255, 255, 255));
  // p.setColor(QPalette::Window, QColor(0, 0, 0));
  // p.setColor(QPalette::Base, QColor(0, 0, 0));
  l1->setPalette(QPalette(QColor(0,0,0)));
  l1->setTextFormat(Qt::RichText);
}

ElementView::~ElementView() {
  cvmap().remove(cv->id()); 
}

QString ElementView::refText() const {
  return ref;
}

QString ElementView::pvText() const {
  return pv;
}

ComponentView const *ElementView::component() const {
  return cv;
}

ComponentView *ElementView::component() {
  return cv;
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
  QString lbl = "<b>";
  bool isNum = ref.mid(1).toDouble()>0;
  int midStart = isNum ? 1 : lbl.indexOf("_");
  int leftLen = isNum ? midStart : midStart - 1;
  if (midStart>0)
    lbl += "<i>" + ref.left(leftLen) + "</i>"
      + "<sub>" + ref.mid(midStart) + "</sub>";
  else
    lbl += ref;
  lbl += "</b>";
  lbl += "  ";
  lbl += pv;
  l1->setText(lbl);
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

