// PartListView.cpp

#include "PartListView.h"
#include "PartList.h"
#include "HtmlDelegate.h"
#include <QItemSelectionModel>
#include <QSet>
#include <QDebug>

PartListView::PartListView(QWidget *parent): QTableView(parent) {
  HtmlDelegate *delegate = new HtmlDelegate(this);
  setItemDelegateForColumn(int(PartList::Column::Name), delegate);
  setSelectionBehavior(SelectRows);
}

PartListView::~PartListView() {
}

void PartListView::setModel(class PartList *pl) {
  QTableView::setModel(pl);
}

PartList *PartListView::model() const {
  return dynamic_cast<PartList *>(QTableView::model());
}

void PartListView::showEvent(QShowEvent *e) {
  QTableView::showEvent(e);
  hideColumn(int(PartList::Column::Id));
  resizeColumnToContents(int(PartList::Column::Name));
  resizeColumnToContents(int(PartList::Column::Value));
  resizeColumnToContents(int(PartList::Column::Vendor));
  resizeColumnToContents(int(PartList::Column::CatNo));
  resizeColumnToContents(int(PartList::Column::Package));
}

void PartListView::resetWidth() {
}

QSet<int> PartListView::selectedElements() const {
  QModelIndexList rows
    = selectionModel()->selectedRows(int(PartList::Column::Id));
  QSet<int> res;
  for (auto const &idx: rows)
    res << idx.data().toInt();
  return res;
}

void PartListView::selectElements(QSet<int> const &set) {
  QMap<int, int> id2row;
  for (int row=0; row<model()->rowCount(); row++) {
    int id
      = model()->data(model()->index(row, int(PartList::Column::Id))).toInt();
    id2row[id] = row;
  }
  QSet<int> old = selectedElements();
  QSet<int> nw;
  for (int id: id2row.keys())
    if (set.contains(id))
      nw << id;

  qDebug() << "plv::se" << set << id2row << old << nw;

  int C = model()->columnCount();
  for (int id: nw-old) {
    int row = id2row[id];
    QItemSelection sel(model()->index(row, 0),
		       model()->index(row, C-1));
    qDebug() << row << sel;
    selectionModel()->select(sel, QItemSelectionModel::Select);
  }
  for (int id: old-nw) {
    int row = id2row[id];
    QItemSelection sel(model()->index(row, 0),
		       model()->index(row, C-1));
    selectionModel()->select(sel, QItemSelectionModel::Deselect);
  }
}
