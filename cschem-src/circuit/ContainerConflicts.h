// ContainerConflicts.h

#ifndef CONTAINERCONFLICTS_H

#define CONTAINERCONFLICTS_H

#include <QStringList>

class ContainerConflicts {
public:
  ContainerConflicts(class Circuit const &, class SymbolLibrary const &);
  QStringList conflicts() const { return issues; }
  /* We report conflicts of multiple kinds:
     - Containers with illformed names
     - Elements with compound names without containers
     - Contents element with names not matching container slots
  */
private:
  QStringList issues;
};

#endif
