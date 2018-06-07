// Modebar.cpp

#include "Modebar.h"
#include <QDebug>

Modebar::Modebar(QWidget *parent): QToolBar("Mode", parent) {
  auto addAct = [this](Mode m, QString lbl, Qt::Key k) {
    actions[m] = addAction(QIcon(":/icons/"+lbl), lbl,
			   [this, m]() { setMode(m); });
    actions[m]->setShortcut(QKeySequence(k));
    actions[m]->setCheckable(true);
  };
  m = Mode::Invalid;
  addAct(Mode::Edit, "Edit", Qt::Key_F1);
  addAct(Mode::PlaceTrace, "Trace", Qt::Key_F2);
  addAct(Mode::PlaceComponent, "Component", Qt::Key_F3);
  addAct(Mode::PlaceHole, "Hole", Qt::Key_F4);
  addAct(Mode::PlacePad, "Pad", Qt::Key_F5);
  addAct(Mode::PlaceText, "Text", Qt::Key_F6);
  addAct(Mode::PlacePlane, "Plane", Qt::Key_F7);
  addAct(Mode::PickupTrace, "Disconnect", Qt::Key_F8);
  setMode(Mode::Edit);
}

Modebar::~Modebar() {
}


void Modebar::setMode(Mode m1) {
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
