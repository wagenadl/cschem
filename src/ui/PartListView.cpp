// PartListView.cpp

#include "PartListView.h"

#include "file/Schem.h"
#include "file/Parts.h"
#include "file/Element.h"
#include "file/Package.h"
#include "file/Circuit.h"

#include <QDebug>

class PLVData {
public:
  PLVData(PartListView *view, Schem *schem):
    view(view),
    schem(schem) { }
  void rebuildRow(int n, Element const &elt);
public:
  PartListView *view;
  Schem *schem;
};

PartListView::PartListView(Schem *schem, QWidget *parent):
  QTableWidget(parent), d(new PLVData(this, schem)) {
  setColumnCount(8);
  setHorizontalHeaderItem(0, new QTableWidgetItem("Part"));
  setHorizontalHeaderItem(1, new QTableWidgetItem("Value"));
  setHorizontalHeaderItem(2, new QTableWidgetItem("Mfg."));
  setHorizontalHeaderItem(3, new QTableWidgetItem("Cat #"));
  setHorizontalHeaderItem(4, new QTableWidgetItem("Vendor"));
  setHorizontalHeaderItem(5, new QTableWidgetItem("Cat #"));
  setHorizontalHeaderItem(6, new QTableWidgetItem("Package"));
  setColumnHidden(7, true);
  sortByColumn(0, Qt::AscendingOrder);
  rebuild();
}

PartListView::~PartListView() {
  delete d;
}

void PartListView::rebuild() {
  int N = 0;
  setRowCount(0);
  setSortingEnabled(false);

  for (Element const &elt: d->schem->circuit().elements()) {
    if (!elt.name().isEmpty() && !elt.isVirtual()
        && elt.type()==Element::Type::Component) {
      setRowCount(N + 1);
      setVerticalHeaderItem(N, new QTableWidgetItem(""));
      setItem(N, 7, new QTableWidgetItem(QString::number(elt.id())));
      d->rebuildRow(N, elt);
      N ++;
    }
  }
  setSortingEnabled(true);
  resizeColumnsToContents();
}

void PLVData::rebuildRow(int n, Element const &elt) {
  qDebug() << "rebuildrow" << n << elt.report();
  auto const &pkgs = schem->parts().packages();
  
  view->setItem(n, 1, new QTableWidgetItem(elt.value()));
  if (pkgs.contains(elt.id())) {
    Package const &pkg(pkgs[elt.id()]);
    view->setItem(n, 2, new QTableWidgetItem(pkg.manufacturer()));
    view->setItem(n, 3, new QTableWidgetItem(pkg.mfgPart()));
    view->setItem(n, 4, new QTableWidgetItem(pkg.vendor()));
    view->setItem(n, 5, new QTableWidgetItem(pkg.partno()));
    view->setItem(n, 6, new QTableWidgetItem(pkg.package()));
  }
  view->setItem(n, 0, new QTableWidgetItem(elt.name()));
  view->item(n, 0)->setFlags(view->item(n, 0)->flags()
                       & ~(Qt::ItemIsEditable));
}

void PartListView::rebuildOne(int id) {
  qDebug() << "rebuildone" << id;
  setSortingEnabled(false);
  int N = rowCount();
  QString txtid = QString::number(id);
  for (int n=0; n<N; n++) {
    if (item(n, 7)->text() == txtid) {
      if (d->schem->circuit().elements().contains(id)) {
        d->rebuildRow(n, d->schem->circuit().element(id));
        setSortingEnabled(true);
        return;
      } else {
        // move to end of table, then remove
        item(n, 7)->setText("zzz");
        sortByColumn(7, Qt::AscendingOrder);
        setSortingEnabled(true);
        setRowCount(N-1);
        sortByColumn(0, Qt::AscendingOrder);
        return;
      }
    }
  }
  
  // the item was not previously there, so we'll add
  setRowCount(N+1);
  setItem(N, 7, new QTableWidgetItem(txtid));
  d->rebuildRow(N, d->schem->circuit().element(id));
  setSortingEnabled(true);    
}
