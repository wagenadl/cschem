// Modebar.cpp

#include "Modebar.h"
#include <QDebug>

Modebar::Modebar(QWidget *parent): QToolBar("Mode", parent) {
  auto addAct = [this](Mode m, QString lbl) {
    actions[m] = addAction(lbl, [this, m]() { setMode(m); });
    actions[m]->setCheckable(true);
  };
  m = Mode::Invalid;
  addAct(Mode::Edit, "Edit");
  addAct(Mode::PlaceTrace, "Trace");
  addAct(Mode::PlaceComponent, "Component");
  addAct(Mode::PlaceHole, "Hole");
  addAct(Mode::PlacePad, "Pad");
  addAct(Mode::PlaceText, "Text");
  addAct(Mode::PlacePlane, "Plane");
  addAct(Mode::PickupTrace, "Disconnect");
  setMode(Mode::Edit);
}

Modebar::~Modebar() {
}


void Modebar::setMode(Mode m1) {
  qDebug() << "setMode" << int(m1);
  if (m1 != m) {
    m = m1;
    for (Mode q: actions.keys()) 
      actions[q]->setChecked(q==m);
    emit modeChanged(m);
  }
}

Mode Modebar::mode() const {
  return m;
}
