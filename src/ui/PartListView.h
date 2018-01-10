// PartListView.h

#ifndef PARTLISTVIEW_H

#define PARTLISTVIEW_H

#include <QTableWidget>

class PartListView: public QTableWidget {
  Q_OBJECT;
public:
  explicit PartListView(class Schem *, QWidget *parent=0);
  ~PartListView();
  PartListView(PartListView const &) = delete;
  PartListView &operator=(PartListView const &) = delete;
public slots:
  void rebuild(); // call when parts are changed externally
  void rebuildOne(int id);
private:
  class PLVData *d;
};

#endif
