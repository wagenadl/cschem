// Modebar.cpp

#include "Modebar.h"
#include <QDebug>

Modebar::Modebar(QWidget *parent): QToolBar("Mode", parent) {
  auto addAct = [this](Mode m, QString lbl, Qt::Key k) {
    actions[m] = addAction(QIcon(":/icons/"+lbl), lbl,
			   [this, m]() { setMode(m); });
    actions[m]->setShortcut(QKeySequence(k));
    actions[m]->setCheckable(true);
    actions[m]->setToolTip(lbl + " (" + QKeySequence(k).toString() +")");
    return actions[m];
  };
  m = Mode::Invalid;
  addAct(Mode::Edit, "Edit", Qt::Key_F1);
  addAct(Mode::PlaceTrace, "Trace", Qt::Key_F2);
  //  addAct(Mode::PlaceComponent, "Component", Qt::Key_F3);
  addAct(Mode::PlaceHole, "Hole", Qt::Key_F3);
  addAct(Mode::PlacePad, "Pad", Qt::Key_F4);
  addAct(Mode::PlaceText, "Text", Qt::Key_F5);
  addAct(Mode::PlaceArc, "Arc", Qt::Key_F6);
  addAct(Mode::PlacePlane, "Plane", Qt::Key_F7);
  addAct(Mode::PickupTrace, "Disconnect", Qt::Key_F8);

  addSeparator();
  a_origin = addAct(Mode::SetIncOrigin, "AbsOrigin", Qt::Key_F11);
  addSeparator();

  a_constr = addAction("Angles",
		       [this]() { setConstraint(!isconstr); });
  QKeySequence ks(Qt::Key_F12);
  a_constr->setShortcut(ks);
  
  setMode(Mode::Edit);
  isconstr = true; // force actual change
  setConstraint(false);
  setAbsInc(false);
}

Modebar::~Modebar() {
}

void Modebar::setConstraint(bool c) {
  if (c == isconstr)
    return;
  QString ic = c ? "Constrain45" : "NoConstrain45";
  a_constr->setIcon(QIcon(":/icons/" + ic));
  QString lbl = c ? "Constrained angles" : "Free angles";
  a_constr->setToolTip(lbl + " (" + a_constr->shortcut().toString()
		       + " toggles)");
  isconstr = c;
  emit constraintChanged(c);
}

bool Modebar::isConstrained() const {
  return isconstr;
}

void Modebar::setMode(Mode m1) {
  for (Mode q: actions.keys()) 
    actions[q]->setChecked(q==m1);
  if (m1!=m) {
    m = m1;
    emit modeChanged(m);
  }
  if (m==Mode::SetIncOrigin)
    setAbsInc(!isinc);
}

void Modebar::setAbsInc(bool inc) {
  isinc = inc;
  QString ic = isinc ? "Inc" : "Abs";
  a_origin->setIcon(QIcon(":/icons/" + ic + "Origin"));
  QString lbl = isinc ? "Incremental" : "Absolute";
  a_origin->setToolTip(lbl + " origin (" + a_origin->shortcut().toString()
		       + " toggles)");
  if (!isinc)
    setMode(Mode::Edit);
  emit originChanged(isinc);
}

bool Modebar::isOriginIncremental() const {
  return isinc;
}

Mode Modebar::mode() const {
  return m;
}
