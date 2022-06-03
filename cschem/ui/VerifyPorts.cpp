// VerifyPorts.cpp

#include "VerifyPorts.h"
#include "circuit/Circuit.h"
#include "svg/Geometry.h"
#include "Scene.h"
#include <QMessageBox>

VerifyPorts::VerifyPorts(Scene *scene, QWidget *parent):
  scene(scene), parent(parent) {
}

void VerifyPorts::run() {
  scene->clearSelection();

  Circuit const &circuit(scene->circuit());

  // First, check for dangling connections.
  // (At the same time, collect connection end points for the next steps.)
  QSet<PinID> allends;
  QSet<QPair<PinID, PinID>> cons;
  for (Connection const &c: circuit.connections) {
    if (c.isDangling()) {
      QMessageBox::information(parent, "CSchem: Verification Failed",
                             "Dangling connections exist.");
      return;
    }
    allends << c.from();
    allends << c.to();
    cons << QPair<PinID, PinID>(c.from(), c.to());
    cons << QPair<PinID, PinID>(c.to(), c.from());
  }

  // Next, check for unconnected ports.
  // (At the same time, count them for the next step.)
  QMap<QString, int> portcount;
  QMap<QString, int> portinstances;
  QSet<int> disconnectedportids;
  QSet<QString> disconnectedportnames;
  for (int e: circuit.elements.keys()) {
    Element const &elt(circuit.elements[e]);
    if (elt.type==Element::Type::Port) {
      QString name = elt.name;
      if (name.isEmpty())
        name = elt.subtype;
      portcount[name]++;
      if (!allends.contains(PinID(elt.id))) {
	disconnectedportnames << name;
	disconnectedportids << elt.id;
      }
    }
  }

  // Any disconnected port instances?
  if (disconnectedportids.size()) {
    scene->selectElements(disconnectedportids);
    QMessageBox::information(parent, "CSchem: Verification Failed",
			     "Disconnected port(s) found: "
			     + QStringList(disconnectedportnames.toList()).join(", ")
			     + ".");
    //    return;
  }

  // Any ports that occur only once?
  QStringList once;
  for (QString k: portcount.keys()) {
    if (portcount[k]==1) {
      once << k;
      scene->addToSelection(portinstances[k]);
    }
  }
  if (!once.isEmpty()) {
    QMessageBox::information(parent, "CSchem: Verification Failed",
			     "The following port(s) occurred only once:\n"
			     + once.join(", ") + "\n"
			     + "This is a sign of a possible error.");
    return;
  }

  // Flood complete all connections
  while (true) {
    QSet<QPair<PinID, PinID>> nw;
    for (QPair<PinID, PinID> const &con1: cons) {
      for (QPair<PinID, PinID> const &con2: cons) {
	if (cons.contains(QPair<PinID, PinID>(con1.first, con2.first))
	    && !cons.contains(QPair<PinID, PinID>(con1.first, con2.second))) {
	  nw << QPair<PinID, PinID>(con1.first, con2.second);
	  nw << QPair<PinID, PinID>(con2.second, con1.first);
	}
      }
    }
    if (nw.size())
      cons += nw;
    else
      break;
  }

  // Let's find all pins and check for close but not connected ones
  QMap<PinID, QPoint> pins;
  Geometry geom(circuit, scene->library());
  for (int e: circuit.elements.keys()) {
    Element const &elt(circuit.elements[e]);
    QMap<QString, QPoint> pos = geom.pinPositions(elt);
    for (QString const &pin: pos.keys())
      pins[PinID(e, pin)] = pos[pin];
  }
  QStringList problems;
  for (PinID const &id1: pins.keys()) {
    for (PinID const &id2: pins.keys()) {
      if (id1!=id2 && (pins[id1] - pins[id2]).manhattanLength() < 10) {
	if (!cons.contains(QPair<PinID, PinID>(id1, id2))) {
	  problems
	    << QString("%1:%2 near %3:%4")
	    .arg(circuit.elements[id1.element()].name)
	    .arg(id1.pin())
	    .arg(circuit.elements[id2.element()].name)
	    .arg(id2.pin());
	  scene->addToSelection(id1.element());
	  scene->addToSelection(id2.element());
	}
      }
    }
  }
  if (!problems.isEmpty()) {
    QMessageBox::information(parent, "CSchem: Verification Failed",
			     "Dangerously close points found: "
			     + problems.join(", ")
			     + ".");
    return;
  }
	  

  // Amazing! All good!
  QMessageBox::information(parent, "CSchem: Verification OK",
			   "All used ports occur at least twice and"
			   " are connected.");
}
