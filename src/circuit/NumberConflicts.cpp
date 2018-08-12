// NumberConflicts.cpp

#include "NumberConflicts.h"
#include "PartNumbering.h"

QStringList NumberConflicts::conflictingNames() const {
  return conflicts;
}
 
NumberConflicts::NumberConflicts(Circuit const &circ): circ(circ) {
  canknown = false;
  mapknown = false;

  QSet<QString> conflicted;

  for (Element const &elt: circ.elements) {
    if (elt.type!=Element::Type::Component)
      continue;
    int id = elt.id;
    QString name = elt.name;
    if (name.isEmpty())
      conflicted << "[anonymous]";
    bool isc = elt.isContainer();
    if (isc) {
      if (name2container.contains(name))
	conflicted << name;
      name2container[name] << id;
    } else {
      if (name2noncont.contains(name))
	conflicted << name;
      name2noncont[name] << id;
    }
  }

  conflicts = conflicted.toList();
}

bool NumberConflicts::canResolve() const {
  if (canknown)
    return can;

  canknown = true; // this will be a fact upon any return from here on down
  can = false; // temporarily...
  
  if (conflicts.isEmpty()) {
    can = true;
    return can;
  }
  if (conflicts.contains("[anonymous]"))
    return false;

  // Drop names that occur only once
  for (QString name: name2container.keys())
    if (name2container[name].size()<2)
      name2container.remove(name);
  for (QString name: name2noncont.keys())
    if (name2noncont[name].size()<2)
      name2noncont.remove(name);

  // Fail if any name is not "well formed"---How would we renumber those?
  for (QString name: name2container.keys())
    if (!PartNumbering::isNameWellFormed(name))
      return false;
  for (QString name: name2noncont.keys())
    if (!PartNumbering::isNameWellFormed(name))
      return false;

  // Fail if any container with conflicting name is nonempty---How
  // would we renumber the contents?
  for (QString name: name2container.keys()) 
    for (int id: name2container[name])
      if (!circ.containedElements(id).isEmpty())
	return false;

  // Fail if any noncontainer with conflicting names is contained---How
  // would we renumber the container?
  for (QString name: name2noncont.keys())
    for (int id: name2noncont[name])
      if (circ.containerOf(id)>0)
	return false;

  can = true;
  return true;
}

QMap<int, QString> const &NumberConflicts::newNames() const {
  if (mapknown)
    return map;

  mapknown = true; // this will be a fact upon any return from here on down
  map.clear(); // preliminarily
    
  if (!canResolve()) {
    return map;
  }

  // Find sets of numbers used for each prefix. Note that we do not touch
  // subnumbers.
  QMap<QString, QSet<int>> usedNumbers;
  for (Element const &e: circ.elements) 
    usedNumbers[e.prefix()] << e.number();

  // FIRSTFREE returns the first positive integer that is not in USEDNUMBERS
  // for a given prefix, and inserts that number into USEDNUMBERS so that it
  // will not be returned again.
  auto firstFree = [&](QString pfx) {
    QSet<int> const &used(usedNumbers[pfx]);
    int first = 1;
    while (used.contains(first))
      first ++;
    usedNumbers[pfx] << first;
    return first;
  };

  // RENUMBER renumbers all but the first of a conflicting set of elements
  auto renumber = [&](QMap<QString, QList<int>> const &eltmap) {
    for (QString name: eltmap.keys()) {
      QList<int> ids = eltmap[name];
      ids.removeFirst();
      QString pfx = PartNumbering::prefix(name);
      for (int id: ids) {
	int newnum = firstFree(pfx);
	map[id] = pfx + QString::number(newnum) + PartNumbering::csuffix(name);
      }
    }
  };

  // Renumber all but the first of a conflicting set of container names.
  // Note that, by construction, these containers are empty, so we don't
  // have to worry about contained elements.
  renumber(name2container);
  // Renumber all but the first of a conflicting set of noncontainer names.
  // Note that, by construction, these elements are not contained in anything,
  // so we don't have to worry about containers.
  renumber(name2noncont);

  return map;
}
