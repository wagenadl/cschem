// PartsListEditor.h

#ifndef PARTSLISTEDITOR_H

#define PARTSLISTEDITOR_H

#include "circuit/Circuit.h"
#include "data/Group.h"

#include <QDialog>

class PartsListEditor: public QDialog {
  Q_OBJECT;
public:
  PartsListEditor(QWidget *parent=0);
  ~PartsListEditor();
  Group const &root() const;
public slots:
  void setCircuit(Circuit const &);
  void setRoot(Group const &);
signals:
  void applyPressed();
private:
  class PLEData *d;
};

#endif
