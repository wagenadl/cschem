// PackagePanel.cpp

#include "PackagePanel.h"
#include "PackageWidget.h"
#include "circuit/Element.h"
#include "svg/PackageLibrary.h"

#include <QLabel>
#include <QDebug>
#include <QVBoxLayout>

class PackagePanelData {
public:
  PackageLibrary const *lib;
  bool freescale;
  double scale;
public:
  QString currentsym;
  QString currentpkg;
  QStringList currentrec;
  int currentpincount;
public:
  QVBoxLayout *layout;
  QLabel *label1;
  PackageWidget *current;
  QLabel *label2;
  QList<PackageWidget *> recommended;
  QLabel *label3;
  QList<PackageWidget *> compatible;
};

PackagePanel::PackagePanel(QWidget *parent):
  QScrollArea(parent), d(new PackagePanelData) {
  d->lib = 0;
  d->scale = 0.2;
  d->freescale = false;
  setAutoFillBackground(true);
  d->layout = new QVBoxLayout;
  d->label1 = new QLabel("Current");
  d->layout->addWidget(d->label1);
  d->current = new PackageWidget;
  d->current->setScale(d->scale);
  d->current->setFreeScaling(d->freescale);
  d->layout->addWidget(d->current);
  d->label2 = new QLabel("Recommended");
  d->layout->addWidget(d->label2);
  d->label3 = new QLabel("Compatible");
  d->layout->addWidget(d->label3);
  setLayout(d->layout);
}

PackagePanel::~PackagePanel() {
  delete d;
}

void PackagePanel::setFreeScaling(bool fs) {
  d->freescale = fs;
  if (fs)
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  else
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
  d->current->setFreeScaling(fs);
  for (auto *w: d->recommended)
    w->setFreeScaling(fs);
  for (auto *w: d->compatible)
    w->setFreeScaling(fs);
  update();
}

bool PackagePanel::isFreeScaling() const {
  return d->freescale;
}

void PackagePanel::setScale(double s) {
  d->scale = s;
  d->current->setScale(s);
  for (auto *w: d->recommended)
    w->setScale(s);
  for (auto *w: d->compatible)
    w->setScale(s);
  update();
}

double PackagePanel::scale() const {
  return d->scale;
}

void PackagePanel::setLibrary(class PackageLibrary const *lib) {
  d->lib = lib;
  d->current->setLibrary(lib);
  for (auto *w: d->recommended)
    w->setLibrary(lib);
  for (auto *w: d->compatible)
    w->setLibrary(lib);
}

void PackagePanel::clear() {
  d->currentsym = "";
  d->currentrec = QStringList();
  d->currentpincount = 0;
  d->current->setPackage("");
  for (auto *w: d->recommended)
    delete w;
  d->recommended.clear();
  for (auto *w: d->compatible)
    delete w;
  d->compatible.clear();
}

void PackagePanel::setElement(Element const &elt) {
  QString sym = elt.symbol();
  QString pkg = elt.info.package;
  qDebug() << "setelement" << sym << d->currentsym;
  if (d->currentsym==sym && d->currentpkg==pkg)
    return;
  d->currentsym = sym;
  d->currentpkg = pkg;
  
  d->current->setPackage(pkg);

  QStringList rec = (d->lib) ? d->lib->recommendedPackages(sym) : QStringList();
  if (rec==d->currentrec)
    return;

  d->currentrec = rec;

  for (auto *w: d->recommended)
    delete w;
  d->recommended.clear();

  // find out where to insert in layout
  int N = d->layout->count();
  int idx = N;
  for (int i=0; i<N; i++) {
    auto *it = d->layout->itemAt(i);
    if (it && it->widget() == d->label3) {
      idx = i;
      break;
    }
  }

  // insert in layout
  for (QString s: rec) {
    auto *w = new PackageWidget();
    w->setLibrary(d->lib);
    w->setScale(d->scale);
    w->setFreeScaling(d->freescale);
    w->setPackage(s);
    d->recommended << w;
    d->layout->insertWidget(idx++, w);
  }
}
