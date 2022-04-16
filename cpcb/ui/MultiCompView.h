// MultiCompView.h

#ifndef MULTICOMPVIEW_H

#define MULTICOMPVIEW_H

#include <QScrollArea>
#include "circuit/Schem.h"
#include "data/Group.h"

class MultiCompView: public QScrollArea {
  Q_OBJECT;
public:
  MultiCompView(QWidget *parent=0);
  ~MultiCompView();
  Schem const &schem() const;
  Group const &root() const;
public slots:
  void setSchem(Schem const &);
  void setRoot(Group const &);
  /* Call setRoot whenever the list of placed components may have changed.
     This determines which of the Elements of the circuit are shown in
     the list. */
  void setScale(double pxPerMil);
  void linkEditor(class Editor *); // for pasting selection only (ugly mech)
private:
  class MCVData *d;
};

#endif
