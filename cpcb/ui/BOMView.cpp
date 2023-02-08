// BOMView.cpp

#include "BOMView.h"
#include "BOM.h"
#include "../cschem/ui/HtmlDelegate.h"
#include <QItemSelectionModel>
#include <QSet>
#include <QDebug>
#include <QHeaderView>
#include <QSortFilterProxyModel>
#include "circuit/PartNumbering.h"
#include <QPainter>
#include <QTextDocument>
#include <QStaticText>

class SortProxy: public QSortFilterProxyModel {
public:
  SortProxy(QObject *parent): QSortFilterProxyModel(parent) {}
  virtual ~SortProxy() {}
  bool lessThan(QModelIndex const &a, QModelIndex const &b) const override;
};

bool SortProxy::lessThan(QModelIndex const &a, QModelIndex const &b) const {
  return PartNumbering::lessThan(a.data().toString(), b.data().toString());
}

class RefHeaderView: public QHeaderView {
public:
  RefHeaderView(Qt::Orientation orientation, QWidget *parent=nullptr);
  QSize sectionSizeFromContents(int logicalIndex) const override;
  void paintSection(QPainter *painter, const QRect &rect,
                    int logicalIndex) const override;
};

RefHeaderView::RefHeaderView(Qt::Orientation orientation,
                                 QWidget *parent):
  QHeaderView(orientation, parent) {
}

QSize RefHeaderView::sectionSizeFromContents(int logicalIndex) const {
  QTextDocument doc;
  doc.setHtml(model()->headerData(logicalIndex, this->orientation(),
                                  Qt::DisplayRole).toString());
  qDebug() << "size" << logicalIndex << doc.size();
  return doc.size().toSize();
}

void RefHeaderView::paintSection(QPainter *painter, const QRect &rect,
                                   int logicalIndex) const {
  /* From https://forum.qt.io/topic/30598 */
painter->save();
QHeaderView::paintSection(painter, rect, logicalIndex);
painter->restore();

//  QStyleOptionHeader opt;
//  initStyleOption(&opt);
//  opt.text = ""; 
//  style()->drawControl(QStyle::CE_Header, &opt, painter, this);
  painter->save();
  // QRect textRect = style()->subElementRect(QStyle::SE_HeaderLabel, &opt, this);
  painter->translate(rect.topLeft());
  QTextDocument doc;
  doc.setHtml(model()->headerData(logicalIndex, this->orientation(),
                                  Qt::DisplayRole).toString());
  //  doc.setDocumentMargin(0);
  qDebug() << "paintsection" << logicalIndex<< rect<<  doc.toPlainText();
  doc.drawContents(painter); // QRect(QPoint(0,0), textRect.size()));
  painter->restore();
}


BOMView::BOMView(QWidget *parent): QTableView(parent) {
  quiet = 0;
  HtmlDelegate *delegate = new HtmlDelegate(this);
  setItemDelegateForColumn(int(BOM::Column::Ref), delegate);
  // setSelectionBehavior(SelectRows);
  //  setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  setSelectionBehavior(SelectItems);
  setSelectionMode(ExtendedSelection);
  sortProxy = new SortProxy(this);
  pl = 0;
  //auto *hdr = new RefHeaderView(Qt::Vertical, this);
  //setVerticalHeader(hdr);

  setDragEnabled(true);
  setAcceptDrops(true);
  setDropIndicatorShown(true);
  
}

BOMView::~BOMView() {
}

bool BOMView::isQuiet() const {
  return quiet > 0;
}

void BOMView::setModel(BOM *pl0) {
  pl = pl0;
  sortProxy->setSourceModel(pl);
  QTableView::setModel(sortProxy);
  sortByColumn(int(BOM::Column::Ref), Qt::AscendingOrder);
  connect(pl0, &BOM::hasLinkedSchematic,
          this, &BOMView::showValueColumn);
}

BOM *BOMView::model() const {
  return pl;
}

void BOMView::showValueColumn(bool x) {
  x = false;
  if (x)
    showColumn(int(BOM::Column::Value));
  else
    hideColumn(int(BOM::Column::Value));
}

void BOMView::showEvent(QShowEvent *e) {
  QTableView::showEvent(e);
  hideColumn(int(BOM::Column::Id));
  hideColumn(int(BOM::Column::Ref));
  resetWidth();
}

void BOMView::resetWidth() {
  //  resizeColumnToContents(int(BOM::Column::Ref));
  resizeColumnToContents(int(BOM::Column::Value));
  resizeColumnToContents(int(BOM::Column::Footprint));
  resizeColumnToContents(int(BOM::Column::Manufacturer));
  resizeColumnToContents(int(BOM::Column::PartNo));
  resizeColumnToContents(int(BOM::Column::Vendor));
  resizeColumnToContents(int(BOM::Column::CatNo));
  resizeColumnToContents(int(BOM::Column::Notes));
}

QSet<int> BOMView::selectedElements() const {
  QModelIndexList cells = selectionModel()->selectedIndexes();
  QSet<int> res;
  for (auto const &idx: cells) {
    int row = idx.row();
    int id = sortProxy->data(sortProxy
                             ->index(row, int(BOM::Column::Id))).toInt();
    res << id;
  }
  return res;
}

void BOMView::selectElements(QSet<int> const &set) {
  quiet++;
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
  quiet--;
}

void BOMView::resizeEvent(QResizeEvent *e) {
  QTableView::resizeEvent(e);
  resetWidth();
}
