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
  bool isQuiet() const;
  void resetWidth();
  void resizeEvent(QResizeEvent *) override;
public slots:
  void cut();
  void copy();
  void paste();
  void deleet();
  void showValueColumn(bool);
private:
  class QSortFilterProxyModel *sortProxy;
  class BOM *pl;
  int quiet;
};

#endif
