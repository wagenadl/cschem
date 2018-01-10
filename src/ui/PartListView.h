// PartListView.h

#ifndef PARTLISTVIEW_H

#define PARTLISTVIEW_H

#include "TextTable.h"

class PartListView: public TextTable {
  Q_OBJECT;
public:
  explicit PartListView(class Schem *, QWidget *parent=0);
  ~PartListView();
  PartListView(PartListView const &) = delete;
  PartListView &operator=(PartListView const &) = delete;
signals:
  void valueEdited(int id);
public slots:
  void rebuild(); // call when parts are changed externally
private slots:
  void internalChange(int r);
private:
  class PLVData *d;
};

#endif
