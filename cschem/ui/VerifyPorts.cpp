// VerifyPorts.cpp

#include "VerifyPorts.h"
#include "circuit/Circuit.h"
#include "Scene.h"
#include <QMessageBox>

VerifyPorts::VerifyPorts(Scene *scene, QWidget *parent):
  scene(scene), parent(parent) {
}

void VerifyPorts::run() {
  Circuit const &circuit(scene->circuit());
  QMap<QString, int> portcount;
  for (int e: circuit.elements.keys()) {
    Element const &elt(circuit.elements[e]);
    if (elt.type==Element::Type::Port) {
      QString name = elt.name;
      if (name.isEmpty())
        name = elt.subtype;
      portcount[name]++;
    }
  }
  qDebug() << "Ports found:";
  for (QString name: portcount.keys())
    qDebug() << "  " << name << ": " << portcount[name];
  qDebug() << "--";

  QStringList once;
  for (QString k: portcount.keys())
    if (portcount[k]==1)
      once << k;

  if (once.isEmpty()) {
    QMessageBox::information(parent, "CSchem: Verification OK",
                             "All used ports occurred at least twice.");
    return;
  }

  QMessageBox::information(parent, "CSchem: Verification Failed",
                           "The following port(s) occurred only once:\n"
                           + once.join(", ") + "\n"
                           + "This is a sign of a possible error.");

  scene->clearSelection();
  for (int e: circuit.elements.keys()) {
    Element const &elt(circuit.elements[e]);
    if (elt.type==Element::Type::Port) {
      QString name = elt.name;
      if (name.isEmpty())
        name = elt.subtype;
      if (once.contains(name))
        scene->addToSelection(e);
    }
  }
}
