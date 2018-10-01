// Modebar.h

#ifndef MODEBAR_H

#define MODEBAR_H

#include <QToolBar>
#include "Mode.h"

class Modebar: public QToolBar {
  Q_OBJECT;
public:
  Modebar(QWidget *parent=0);
  virtual ~Modebar();
  Mode mode() const;
  bool isConstrained() const;
  bool isOriginIncremental() const;
public slots:
  void setMode(Mode);
  void setConstraint(bool);
  void setAbsInc(bool);
signals:
  void modeChanged(Mode);
  void constraintChanged(bool);
  void originChanged(bool);
private:
  Mode m;
  QMap<Mode, QAction *> actions;
  bool isconstr;
  QAction *a_constr;
  bool isinc;
  QAction *a_origin;
};

#endif
