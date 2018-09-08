// Pinmapper.h

#ifndef PINMAPPER_H

#define PINMAPPER_H

#include <QStringList>
#include <QSharedPointer>

class PinMapper {
public:
  PinMapper(QString ref, QStringList circuitnames, QStringList pcbnames);
  bool isComplete() const; /* True if every circuit name maps to a pcb name.
			      The converse might not be true if the component
			      has extra pins. Pins named "nc" are ignored. */
  bool isContradictory() const;
  QString pcbNameToCircuit(QString) const;
  QString circuitNameToPCB(QString) const;
  QString improvePCBName(QString) const;
  /* If mapping contains circ:"K" or circ:"K/2" and pcb:"3/K", then
     improvePCBName("3") will return "3/K". */
  bool setCircuitNameToPCB(QString circuitname, QString pcbname);
  // true if OK, i.e., if both circuitname and pcbname are known to exist
private:
  QSharedPointer<class PMData >d;
};

#endif
