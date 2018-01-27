// PartListView.cpp

#include "PartListView.h"
#include "PartList.h"
#include "HtmlDelegate.h"

PartListView::PartListView(QWidget *parent): QTableView(parent) {
  HtmlDelegate *delegate = new HtmlDelegate(this);
  setItemDelegateForColumn(int(PartList::Column::Name), delegate);
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
