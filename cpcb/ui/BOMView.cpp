// BOMView.cpp

#include "BOMView.h"
#include "BOM.h"
#include "../cschem/ui/HtmlDelegate.h"
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


BOMView::BOMView(QWidget *parent): QTableView(parent) {
  HtmlDelegate *delegate = new HtmlDelegate(this);
  setItemDelegateForColumn(int(BOM::Column::Ref), delegate);
  setSelectionBehavior(SelectRows);
  setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  sortProxy = new SortProxy(this);
  pl = 0;
}

BOMView::~BOMView() {
}

void BOMView::setModel(BOM *pl0) {
  pl = pl0;
  sortProxy->setSourceModel(pl);
  QTableView::setModel(sortProxy);
  sortByColumn(int(BOM::Column::Ref), Qt::AscendingOrder);
}

BOM *BOMView::model() const {
  return pl;
}

void BOMView::showEvent(QShowEvent *e) {
  QTableView::showEvent(e);
  hideColumn(int(BOM::Column::Id));
  resetWidth();
}

void BOMView::resetWidth() {
  resizeColumnToContents(int(BOM::Column::Ref));
  resizeColumnToContents(int(BOM::Column::Value));
  resizeColumnToContents(int(BOM::Column::Package));
  resizeColumnToContents(int(BOM::Column::PartNo));
  setColumnWidth(int(BOM::Column::Notes),
                 viewport()->width()
                 - columnWidth(int(BOM::Column::Ref))
                 - columnWidth(int(BOM::Column::Package))
                 - columnWidth(int(BOM::Column::PartNo))
                 - columnWidth(int(BOM::Column::Value)));
}

QSet<int> BOMView::selectedElements() const {
  QModelIndexList rows
    = selectionModel()->selectedRows(int(BOM::Column::Id));
  QSet<int> res;
  for (auto const &idx: rows)
    res << idx.data().toInt();
  return res;
}

void BOMView::selectElements(QSet<int> const &set) {
  QMap<int, int> id2row;
  for (int row=0; row<model()->rowCount(); row++) {
    int id
      = sortProxy->data(sortProxy
                        ->index(row, int(BOM::Column::Id))).toInt();
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

void BOMView::resizeEvent(QResizeEvent *e) {
  QTableView::resizeEvent(e);
  resetWidth();
}
