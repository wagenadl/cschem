// PartListView.h

#ifndef PARTLISTVIEW_H

#define PARTLISTVIEW_H

#include <QTableView>

class PartListView: public QTableView {
  Q_OBJECT;
public:
  PartListView(QWidget *parent=0);
  virtual ~PartListView();
  void setModel(class PartList *);
  PartList *model() const;
  void showEvent(QShowEvent *) override;
  QSet<int> selectedElements() const;
  void selectElements(QSet<int> const &);
  void resetWidth();
  void resizeEvent(QResizeEvent *) override;
private:
  class QSortFilterProxyModel *sortProxy;
  class PartList *pl;
};

#endif
