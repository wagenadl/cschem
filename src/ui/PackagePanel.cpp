// PackagePanel.cpp

#include "PackagePanel.h"
#include "PackageWidget.h"
#include "circuit/Element.h"
#include "svg/PackageLibrary.h"

#include <QLabel>
#include <QDebug>
#include <QVBoxLayout>
#include <QPainter>


class BackgroundWidget: public QWidget {
public:
  BackgroundWidget(QWidget *parent=0): QWidget(parent) {
    scale = 0.3;
    grid = 100; // mils
    major = 5;
  }
  void setScale(double pixpermil) {
    scale = pixpermil;
    update();
  }
  void setGridSpacing(double mil, int majorival) {
    grid = mil;
    major = majorival;
    update();
  }
protected:
  void paintEvent(QPaintEvent *) override {
    QPainter ptr;
    ptr.begin(this);
    int w = width();
    int h = height();
    ptr.fillRect(QRect(0,0,w,h), QColor(255, 255, 255));
    int x0 = w/2;
    int y0 = h/2;
    ptr.setPen(QPen(QColor(192, 192, 192), 2));
    ptr.drawLine(0, y0, w, y0);
    for (double y = major*grid*scale; y<y0; y += major*grid*scale) {
      ptr.drawLine(0, y0-y, w, y0-y);
      ptr.drawLine(0, y0+y, w, y0+y);
    }
    ptr.drawLine(x0, 0, x0, h);
    for (double x = major*grid*scale; x<x0; x += major*grid*scale) {
      ptr.drawLine(x0+x, 0, x0+x, h);
    }
    ptr.setPen(QPen(QColor(220, 220, 220), 1));
    for (double y = grid*scale; y<y0; y += grid*scale) {
      ptr.drawLine(0, y0-y, w, y0-y);
      ptr.drawLine(0, y0+y, w, y0+y);
    }
    ptr.drawLine(x0, 0, x0, h);
    for (double x = grid*scale; x<x0; x += grid*scale) {
      ptr.drawLine(x0-x, 0, x0-x, h);
      ptr.drawLine(x0+x, 0, x0+x, h);
    }
    ptr.end();
  }
private:
  double scale;
  double grid;
  int major;
};

class PackagePanelData {
public:
  PackageLibrary const *lib;
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
  BackgroundWidget *bg;
};

PackagePanel::PackagePanel(QWidget *parent):
  QScrollArea(parent), d(new PackagePanelData) {
  d->lib = 0;
  d->scale = 0.3;
  setAutoFillBackground(true);
  setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

  d->bg = new BackgroundWidget();
  d->bg->setScale(0.3);
  setWidget(d->bg);
  setWidgetResizable(true);
  
  d->layout = new QVBoxLayout;
  d->label1 = new QLabel("Current");
  QFont f = d->label1->font();
  f.setStyle(QFont::StyleItalic);
  d->label1->setFont(f);
  d->layout->addWidget(d->label1);
  d->current = new PackageWidget;
  connect(d->current, &PackageWidget::pressed,
          this, &PackagePanel::press);
  d->current->setScale(d->scale);
  d->current->setFreeScaling(false);
  d->layout->addWidget(d->current);
  d->label2 = new QLabel("Recommended");
  d->label2->setFont(f);
  d->layout->addWidget(d->label2);
  d->label3 = new QLabel("Compatible");
  d->label3->setFont(f);
  d->layout->addWidget(d->label3);
  d->layout->addStretch();
  widget()->setLayout(d->layout);
}

PackagePanel::~PackagePanel() {
  delete d;
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
    w->setFreeScaling(false);
    w->setPackage(s);
    d->recommended << w;
    d->layout->insertWidget(idx++, w);
  }
}

void PackagePanel::press(QString s) {
  emit pressed(s);
}
    
