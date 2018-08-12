// NumberConflicts.h

#ifndef NUMBERCONFLICTS_H

#define NUMBERCONFLICTS_H

#include <QString>
#include <QMap>
#include <QList>
#include <QStringList>
#include "Circuit.h"

class NumberConflicts {
public:
  NumberConflicts(Circuit const &);
  QStringList conflictingNames() const;
  bool canResolve() const;
  QMap<int, QString> const &newNames() const;
private:
  Circuit const &circ;
  QStringList conflicts;
  mutable bool can;
  mutable QMap<int, QString> map;
  mutable bool canknown;
  mutable bool mapknown;
  mutable QMap<QString, QList<int>> name2container;
  mutable QMap<QString, QList<int>> name2noncont;
};

#endif
