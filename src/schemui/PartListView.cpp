// PartListView.cpp

#include "PartListView.h"

#include "circuit/Schem.h"
#include "circuit/Element.h"
#include "circuit/Circuit.h"
#include "circuit/PartNumbering.h"

#include <QDebug>

enum Columns {
  COL_Ref = 0,
  COL_Value,
  COL_Vendor,
  COL_PartNo,
  COL_Notes,
  COL_ID,
  COL_N,
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
  setColumnHeader(COL_Ref, "Ref.");
  setColumnHeader(COL_Value, "Value");
  setColumnHeader(COL_Vendor, "Vendor");
  setColumnHeader(COL_PartNo, "Cat #");
  setColumnHeader(COL_Notes, "Notes");
  setColumnHidden(COL_ID, true);
  sortByColumn(COL_Ref, Qt::AscendingOrder);
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
  resetWidth();
  d->rebuilding = false;
}

void PartListView::resetWidth() {
  resizeColumnsToContents();
  int w = 0;
  for (int c=0; c<COL_ID; c++) 
    w += columnWidth(c);

  QWidget *p = parentWidget(); // that's the containing dock
  if (p)
    p = p->parentWidget(); // get to main window
  if (p) {
    int pw = p->width();
    if (w>pw/3)
      w = pw/3;
  }

  setMinimumWidth(w);
}

void PLVData::rebuildRow(int n, Element const &elt) {
  view->setText(n, COL_Value, elt.value());
  view->setText(n, COL_Vendor, elt.info().vendor);
  view->setText(n, COL_PartNo, elt.info().partno);
  view->setText(n, COL_Notes, elt.info().notes);
  QString name = elt.name();
  name.replace(".1", "");   
  view->setText(n, 0, name);
  view->item(n, 0)->setFlags(view->item(n, 0)->flags()
                       & ~(Qt::ItemIsEditable));
}

void PartListView::internalChange(int n) {
  if (d->rebuilding)
    return;

  Circuit circ(d->schem->circuit());
  int id = text(n, COL_ID).toInt();
  if (circ.elements().contains(id)) {
    QString val0 = text(n, COL_Value);
    QString vendor = text(n, COL_Vendor);
    QString partno = text(n, COL_PartNo);
    QString notes = text(n, COL_Notes);

    Element elt = circ.element(id);
    
    QString val = PartNumbering::prettyValue(val0, elt.name());
    
    if (elt.value() != val
        || elt.info().vendor != vendor
        || elt.info().partno != partno
        || elt.info().notes != notes) {
      elt.setValue(val);
      Element::Info info;
      info.vendor = vendor;
      info.partno = partno;
      info.notes = notes;
      elt.setInfo(info);
      circ.insert(elt);
      d->schem->setCircuit(circ);
      emit valueEdited(id);
    }
    if (val != val0)
      setText(n, COL_Value, val);
  }
}

QList<QStringList> PartListView::partList() const {
  QList<QStringList> res;
  QStringList hdr;
  hdr << "Ref." << "Value" << "Vendor" << "Cat.#" << "Notes";
  res << hdr;
  int N = rowCount();
  for (int n=0; n<N; n++) {
    QStringList line;
    line << text(n, COL_Ref) << text(n, COL_Value)
	 << text(n, COL_Vendor) << text(n, COL_PartNo)
	 << text(n, COL_Notes);
    res << line;
  }
  return res;
}
