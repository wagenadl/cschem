// ContainerConflicts.cpp

#include "ContainerConflicts.h"
#include "Circuit.h"
#include "../svg/SymbolLibrary.h"
#include <algorithm>

ContainerConflicts::ContainerConflicts(Circuit const &circ,
				       SymbolLibrary const &lib) {
  int anon = 0; // count of anonymous elements
  for (Element const &elt: circ.elements) {
    if (elt.type != Element::Type::Component)
      continue;
    if (elt.isContainer()) {
      if (!elt.isNameWellFormed())
	issues << "Container with illformed name: " + elt.name;
      else if (!elt.csuffix().isEmpty())
	issues << "Container with compund name: " + elt.name
	  + " (containers should have simple names like " + elt.cname() + ")";
    } else {
      if (elt.name.isEmpty()) {
	anon++;
      } else {
	int cid = circ.containerOf(elt.id);
	if (cid>0) {
	  Element const &cont(circ.elements[cid]);
	  Symbol const &symbol(lib.symbol(cont.symbol()));
	  if (symbol.isValid()) {
	    if (elt.csuffix().isEmpty()) {
	      if (symbol.slotCount()>1)
		issues << "Element " + elt.name + " contained in container"
		  " with more than one slot. Should be named something like "
		  + elt.name + ".1";
	    } else {
	      if (!symbol.containerSlots().contains(elt.subNumber()))
		issues << "Element " + elt.name + " contained in container"
		  " without a slot numbered " + QString::number(elt.subNumber());
	    }
	  } else {
	    issues << "Missing container for " + elt.name;
	  }
	}
      }
    }
  }
  std::sort(issues.begin(), issues.end());
}
