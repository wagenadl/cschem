// PackagePanel.cpp

#include "PackagePanel.h"
#include "PackageWidget.h"
#include "circuit/Element.h"
#include "svg/PackageLibrary.h"
#include "ui/PackageBackground.h"

#include <QLabel>
#include <QDebug>
#include <QVBoxLayout>
#include <QPainter>
#include <math.h>

class PackageLabel: public QWidget {
public:
  PackageLabel(QString lbl, PackagePanelData *d, bool top=false):
    lbl(lbl), d(d), top(top) {}
  QSize sizeHint() const override;
  QSize minimumSizeHint() const override;
protected:
  void paintEvent(QPaintEvent *) override;
private:
  QString lbl;
  PackagePanelData *d;
  bool top;
};

class PackagePanelData {
public:
  PackageLibrary const *lib;
  double scale;
  double grid;
  int major;
public:
  QString currentsym;
  QString currentpkg;
  QStringList currentrec;
  int currentpincount;
public:
  QVBoxLayout *layout;
  PackageLabel *label1;
  PackageWidget *current;
  PackageLabel *label2;
  QList<PackageWidget *> recommended;
  PackageLabel *label3;
  QList<PackageWidget *> compatible;
  PackageWidget *filler;
};

QSize PackageLabel::minimumSizeHint() const {
  return sizeHint();
}

QSize PackageLabel::sizeHint() const {
  double scl =  d->grid*d->scale;
  return (scl*QSizeF(8, top ? 2 : 3)).toSize();
}

void PackageLabel::paintEvent(QPaintEvent *) {
  QPainter ptr(this);

  ptr.fillRect(QRect(QPoint(0,0), size()), QColor(255,255,255));

  // draw minor grid
  ptr.setPen(QPen(QColor(220, 220, 220), 1));
  int h = height();
  int w = width();
  double gridsp = d->grid * d->scale;
  for (double x=0; x<w; x+=gridsp)
    ptr.drawLine(x, 0, x, h);
  for (double y=0; y<h; y+=gridsp)
    ptr.drawLine(0, y, w, y);

  ptr.setPen(QPen(QColor(192, 192, 192), 2));
  int x0 = 2; // nx
  while (x0>0)
    x0 -= d->major;
  x0 += d->major;
  for (double x=x0*gridsp; x<w; x+=d->major*gridsp)
    ptr.drawLine(x, 0, x, h);
  ptr.setPen(QPen(QColor(128, 128, 128), 2));
  ptr.drawLine(0, top ? 1 : gridsp, w, top ? 1 : gridsp);

  ptr.setPen(QPen(QColor(0, 0, 0)));
  ptr.drawText(0.8*gridsp, (top ? 1.8 : 2.8)*gridsp, lbl);
}  


PackagePanel::PackagePanel(QWidget *parent):
  QScrollArea(parent), d(new PackagePanelData) {
  d->lib = 0;
  d->scale = 0.3;
  d->grid = 100;
  d->major = 5;
  setAutoFillBackground(true);
  QPalette p = palette();
  p.setColor(QPalette::Window, QColor(255, 255, 255));
  setPalette(p);
  setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

  setWidget(new QWidget());
  setWidgetResizable(true);
  
  d->layout = new QVBoxLayout;
  d->layout->setSpacing(0);
  d->layout->setMargin(0);
  d->label1 = new PackageLabel("Current", d, true);
  //d->label1->setFrameStyle(QFrame::Panel | QFrame::Raised);
  //d->label1->setLineWidth(4);
  QFont f = d->label1->font();
  f.setStyle(QFont::StyleItalic);
  d->label1->setFont(f);
  d->layout->addWidget(d->label1);
  d->current = new PackageWidget;
  connect(d->current, &PackageWidget::pressed,
          this, &PackagePanel::press);
  d->current->setScale(d->scale);
  d->current->setGrid(d->grid, d->major);
  d->layout->addWidget(d->current);
  d->label2 = new PackageLabel("Recommended", d);
  d->label2->setFont(f);
  d->layout->addWidget(d->label2);
  d->label3 = new PackageLabel("Compatible", d);
  d->label3->setFont(f);
  d->layout->addWidget(d->label3);
  d->filler = new PackageWidget;
  d->filler->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  d->filler->setScale(d->scale);
  d->filler->setGrid(d->grid, d->major);
  d->layout->addWidget(d->filler);
  widget()->setLayout(d->layout);
}

PackagePanel::~PackagePanel() {
  delete d;
}

void PackagePanel::setScale(double s) {
  d->scale = s;
  d->current->setScale(s);
  d->filler->setScale(s);
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
  if (d->currentsym==sym && d->currentpkg==pkg)
    return;
  d->currentsym = sym;
  d->currentpkg = pkg;
  
  d->current->setPackage(pkg);

  QStringList rec = (d->lib) ? d->lib->recommendedPackages(sym) : QStringList();
  int idx1 = rec.indexOf(pkg);
  if (idx1>=0)
    rec.removeAt(idx1);
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
    connect(w, &PackageWidget::pressed,
            this, &PackagePanel::press);
    w->setLibrary(d->lib);
    w->setScale(d->scale);
    w->setGrid(d->grid, d->major);
    w->setPackage(s);
    d->recommended << w;
    d->layout->insertWidget(idx++, w);
  }
}

void PackagePanel::press(QString s) {
  emit pressed(s);
}
    
