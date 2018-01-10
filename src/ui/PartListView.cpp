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
    schem(schem),
    rebuilding(false) { }
  void rebuildRow(int n, Element const &elt);
public:
  PartListView *view;
  Schem *schem;
  bool rebuilding;
};

PartListView::PartListView(Schem *schem, QWidget *parent):
  TextTable(parent), d(new PLVData(this, schem)) {
  setColumnCount(8);
  setColumnHeader(0, "Part");
  setColumnHeader(1, "Value");
  setColumnHeader(2, "Mfg.");
  setColumnHeader(3, "Cat #");
  setColumnHeader(4, "Vendor");
  setColumnHeader(5, "Cat #");
  setColumnHeader(6, "Package");
  setColumnHidden(7, true);
  sortByColumn(0, Qt::AscendingOrder);
  connect(this, &PartListView::cellChanged,
          this, &PartListView::internalChange);
  rebuild();
}

PartListView::~PartListView() {
  delete d;
}

void PartListView::rebuild() {
  d->rebuilding = true;
  int N = 0;
  setRowCount(0);
  setSortingEnabled(false);

  for (Element const &elt: d->schem->circuit().elements()) {
    if (!elt.name().isEmpty() && !elt.isVirtual()
        && elt.type()==Element::Type::Component) {
      setRowCount(N + 1);
      setRowHeader(N, "");
      setText(N, 7, QString::number(elt.id()));
      d->rebuildRow(N, elt);
      N ++;
    }
  }
  setSortingEnabled(true);
  resizeColumnsToContents();
  d->rebuilding = false;
}

void PLVData::rebuildRow(int n, Element const &elt) {
  qDebug() << "rebuildrow" << n << elt.report();
  auto const &pkgs = schem->parts().packages();
  
  view->setText(n, 1, elt.value());
  if (pkgs.contains(elt.id())) {
    Package const &pkg(pkgs[elt.id()]);
    view->setText(n, 2, pkg.manufacturer());
    view->setText(n, 3, pkg.mfgPart());
    view->setText(n, 4, pkg.vendor());
    view->setText(n, 5, pkg.partno());
    view->setText(n, 6, pkg.package());
  }
  view->setText(n, 0, elt.name());
  view->item(n, 0)->setFlags(view->item(n, 0)->flags()
                       & ~(Qt::ItemIsEditable));
}

void PartListView::internalChange(int n) {
  if (d->rebuilding)
    return;

  qDebug() << "PLV: internalchange" << n;
  
  Circuit circ(d->schem->circuit());
  int id = text(n, 7).toInt();
  if (circ.elements().contains(id)) {
    QString val = text(n, 1);
    Element elt = circ.element(id);
    if (elt.name().mid(1).toInt()>0) {
      if (elt.name().startsWith("R") && val.endsWith("."))
        val = val.left(val.size() - 1) + tr("Ω");
      else if (elt.name().startsWith("C") || elt.name().startsWith("L"))
        val = val.replace("u", tr("μ"));
    }
    
    if (elt.value() != val) {
      elt.setValue(val);
      circ.insert(elt);
      d->schem->setCircuit(circ);
      emit valueEdited(id);
    }

    QString mfg = text(n, 2);
    QString mfgcat = text(n, 3);
    QString vend = text(n, 4);
    QString vendcat = text(n, 5);
    QString pkg = text(n, 6);
    
    if (mfg.isEmpty() && mfgcat.isEmpty()
        && vend.isEmpty() && vendcat.isEmpty()
        && pkg.isEmpty()) {
      // don't need a <package>
      Parts parts = d->schem->parts();
      parts.packages().remove(id);
      d->schem->setParts(parts);
    } else {
      Package package;
      package.setId(id);
      package.setManufacturer(mfg);
      package.setMfgPart(mfgcat);
      package.setVendor(vend);
      package.setPartno(vendcat);
      package.setPackage(pkg);
      Parts parts = d->schem->parts();
      parts.packages()[id] = package;
      d->schem->setParts(parts);
    }
  }
}
