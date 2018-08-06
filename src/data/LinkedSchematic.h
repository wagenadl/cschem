// LinkedSchematic.h

#ifndef LINKEDSCHEMATIC_H

#define LINKEDSCHEMATIC_H

#include <QObject>
#include <QString>

#include "../../cschem/src/circuit/Schem.h"
#include "../../cschem/src/circuit/Circuit.h"
#include "../../cschem/src/circuit/Net.h"

class LinkedSchematic: public QObject {
  Q_OBJECT;
public:
  LinkedSchematic(QObject *parent=0);
  void link(QString fn);
  void unlink();
  ~LinkedSchematic();
  bool isValid() const;
  Schem schematic() const;
  Circuit circuit() const;
  QList<Net> nets() const;
signals:
  void reloaded();
private:
  LinkedSchematic(LinkedSchematic const &) = delete;
  LinkedSchematic &operator=(LinkedSchematic const &) = delete;
private:
  class LSData *d;
};

#endif
