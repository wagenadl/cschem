// FindSym.cpp

#include "FindSym.h"

#include "ui/Scene.h"
#include "svg/Symbol.h"
#include <QInputDialog>
#include <QMessageBox>

FindSym::FindSym(Scene *scene, QWidget *parent): scene(scene), parent(parent) {
}

bool FindSym::run() {
  scene->clearSelection();
  
  QString key = QInputDialog::getText(parent, "Find symbol",
                                      "Ref. or value:",
                                      QLineEdit::Normal);
  if (key.isEmpty())
    return false;

  key = key.toLower();
  Circuit const &circuit(scene->circuit());
  
  int got = -1;
  for (int e: circuit.elements.keys()) {
    Element const &elt(circuit.elements[e]);
    if (elt.name.toLower()==key || elt.value.toLower()==key) {
      got = e;
      break;
    }
  }
  if (got<0) { // look for partial values
    for (int e: circuit.elements.keys()) {
      Element const &elt(circuit.elements[e]);
      if (elt.name.toLower().contains(key)) {
        got = e;
        break;
      }
    }
  } 

  if (got>=0) {
    scene->addToSelection(got);
    return true;
  } else {
    QMessageBox::information(parent, "Find", "Not found");
    return false;
  }
}
