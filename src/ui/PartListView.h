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
  QList<QStringList> partList() const;
  /* The first string list contains the headers, subsequent ones are
     one list per row, sorted in current display order. */
signals:
  void valueEdited(int id);
public slots:
  void rebuild(); // call when parts are changed externally
  void resetWidth();
private slots:
  void internalChange(int r);
private:
  class PLVData *d;
};

#endif
