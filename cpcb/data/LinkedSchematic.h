// LinkedSchematic.h

#ifndef LINKEDSCHEMATIC_H

#define LINKEDSCHEMATIC_H

#include <QObject>
#include <QString>

#include "circuit/Schem.h"
#include "circuit/Circuit.h"
#include "LinkedNet.h"

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
  QList<LinkedNet> nets() const;
  Nodename pinAlias(Nodename const &) const;
public slots:
  void reload();
signals:
  void reloaded();
private:
  LinkedSchematic(LinkedSchematic const &) = delete;
  LinkedSchematic &operator=(LinkedSchematic const &) = delete;
private:
  class LSData *d;
};

#endif
