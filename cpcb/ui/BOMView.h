// BOMView.h

#ifndef BOMVIEW_H

#define BOMVIEW_H

#include <QTableView>

class BOMView: public QTableView {
  Q_OBJECT;
public:
  BOMView(QWidget *parent=0);
  virtual ~BOMView();
  void setModel(class BOM *);
  BOM *model() const;
  void showEvent(QShowEvent *) override;
  QSet<int> selectedElements() const;
  void selectElements(QSet<int> const &);
  void resetWidth();
  void resizeEvent(QResizeEvent *) override;
private:
  class QSortFilterProxyModel *sortProxy;
  class BOM *pl;
};

#endif
