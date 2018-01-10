// PartListView.cpp

#include "PartListView.h"

#include "file/Schem.h"
#include "file/Parts.h"
#include "file/Element.h"
#include "file/Package.h"
#include "file/Circuit.h"

#include <QDebug>

enum Columns {
  COL_Part = 0,
  COL_Value,
  COL_Vendor,
  COL_Partno,
  COL_Notes,
  COL_ID,
  COL_N
};

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
  setColumnCount(COL_N);
  setColumnHeader(COL_Part, "Part");
  setColumnHeader(COL_Value, "Value");
  setColumnHeader(COL_Vendor, "Vendor");
  setColumnHeader(COL_Partno, "Cat #");
  setColumnHeader(COL_Notes, "Notes");
  setColumnHidden(COL_ID, true);
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
    QString name = elt.name();
    bool secondary = name.contains(".") && !name.endsWith(".1");
    if (elt.type()==Element::Type::Component
        && !elt.name().isEmpty() && !secondary) {
      setRowCount(N + 1);
      setRowHeader(N, "");
      setText(N, COL_ID, QString::number(elt.id()));
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
  
  view->setText(n, COL_Value, elt.value());
  if (pkgs.contains(elt.id())) {
    Package const &pkg(pkgs[elt.id()]);
    view->setText(n, COL_Vendor, pkg.vendor());
    view->setText(n, COL_Partno, pkg.partno());
    view->setText(n, COL_Notes, pkg.notes());
  }
  QString name = elt.name();
  name.replace(".1", "");   
  view->setText(n, COL_Part, name);
  view->item(n, 0)->setFlags(view->item(n, 0)->flags()
                       & ~(Qt::ItemIsEditable));
}

void PartListView::internalChange(int n) {
  if (d->rebuilding)
    return;

  qDebug() << "PLV: internalchange" << n;
  
  Circuit circ(d->schem->circuit());
  int id = text(n, COL_ID).toInt();
  if (circ.elements().contains(id)) {
    QString val = text(n, COL_Value);
    Element elt = circ.element(id);
    if (elt.name().mid(1).toDouble()>0) {
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

    QString vend = text(n, COL_Vendor);
    QString vendcat = text(n, COL_Partno);
    QString notes = text(n, COL_Notes);
    
    if (vend.isEmpty() && vendcat.isEmpty()
        && notes.isEmpty()) {
      // don't need a <package>
      Parts parts = d->schem->parts();
      parts.removePackage(id);
      d->schem->setParts(parts);
    } else {
      Package package;
      package.setId(id);
      package.setVendor(vend);
      package.setPartno(vendcat);
      package.setNotes(notes);
      Parts parts = d->schem->parts();
      parts.insert(package);
      qDebug() << "parts changed" << parts.packages().size() << id;
      d->schem->setParts(parts);
    }
  }
}
