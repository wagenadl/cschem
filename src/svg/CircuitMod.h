// CircuitMod.h

#ifndef CIRCUITMOD_H

#define CIRCUITMOD_H

#include <QSet>

class CircuitMod {
public:
  CircuitMod(class Circuit const &, class PartLibrary const *);
  CircuitMod(CircuitMod const &) = delete;
  CircuitMod &operator=(CircuitMod const &) = delete;
  ~CircuitMod();
  void deleteElement(int id);
  void deleteConnection(int id);
  bool removePointlessJunction(int id); // true if removed
  void deleteAnyDuplicatesOfConnection(int id);
public:
  QSet<int> affectedConnections() const; // new, modified, or deleted
  QSet<int> affectedElements() const; // new, modified, or deleted
  Circuit const &circuit() const;
private:
  class CircuitModData *d;
};

#endif
