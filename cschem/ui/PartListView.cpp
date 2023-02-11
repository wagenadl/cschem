// PartListView.cpp

#include "PartListView.h"
#include "PartList.h"
#include "HtmlDelegate.h"
#include <QItemSelectionModel>
#include <QSet>
#include <QDebug>
#include <QSortFilterProxyModel>
#include "circuit/PartNumbering.h"

class SortProxy: public QSortFilterProxyModel {
public:
  SortProxy(QObject *parent): QSortFilterProxyModel(parent) {}
  virtual ~SortProxy() {}
  bool lessThan(QModelIndex const &a, QModelIndex const &b) const override;
};

bool SortProxy::lessThan(QModelIndex const &a, QModelIndex const &b) const {
  return PartNumbering::lessThan(a.data().toString(), b.data().toString());
}


PartListView::PartListView(QWidget *parent): QTableView(parent) {
  HtmlDelegate *delegate = new HtmlDelegate(this);
  setItemDelegateForColumn(int(PartList::Column::Name), delegate);
  setSelectionBehavior(SelectRows);
  setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  sortProxy = new SortProxy(this);
  pl = 0;
}

PartListView::~PartListView() {
}

void PartListView::setModel(PartList *pl0) {
  pl = pl0;
  sortProxy->setSourceModel(pl);
  QTableView::setModel(sortProxy);
  sortByColumn(int(PartList::Column::Name), Qt::AscendingOrder);
  // setSortingEnabled(true); // this enables user choice in sorting behavior
  hideColumn(int(PartList::Column::Id));
}

PartList *PartListView::model() const {
  return pl;
}

void PartListView::showEvent(QShowEvent *e) {
  QTableView::showEvent(e);
  hideColumn(int(PartList::Column::Id));
  resetWidth();
}

void PartListView::resetWidth() {
  resizeColumnToContents(int(PartList::Column::Name));
  resizeColumnToContents(int(PartList::Column::Value));
  setColumnWidth(int(PartList::Column::Notes),
                 viewport()->width()
                 - columnWidth(int(PartList::Column::Name))
                 - columnWidth(int(PartList::Column::Value)));
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
      = sortProxy->data(sortProxy
                        ->index(row, int(PartList::Column::Id))).toInt();
    id2row[id] = row;
  }
  QSet<int> old = selectedElements();
  QSet<int> nw;
  for (int id: id2row.keys())
    if (set.contains(id))
      nw << id;

  int C = sortProxy->columnCount();
  for (int id: nw-old) {
    int row = id2row[id];
    QItemSelection sel(sortProxy->index(row, 0),
		       sortProxy->index(row, C-1));
    selectionModel()->select(sel, QItemSelectionModel::Select);
  }
  for (int id: old-nw) {
    int row = id2row[id];
    QItemSelection sel(sortProxy->index(row, 0),
		       sortProxy->index(row, C-1));
    selectionModel()->select(sel, QItemSelectionModel::Deselect);
  }
}

void PartListView::resizeEvent(QResizeEvent *e) {
  QTableView::resizeEvent(e);
  resetWidth();
}
