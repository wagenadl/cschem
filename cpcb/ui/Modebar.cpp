// Modebar.cpp

#include "Modebar.h"
#include <QDebug>

Modebar::Modebar(QWidget *parent): QToolBar("Mode", parent) {
  auto addAct2 = [this](Mode m, QString ic, QString lbl, Qt::Key k) {
    QIcon icon(":/icons/" + ic);
        qDebug() << "icon" << ic << "sizes" << icon.availableSizes();
    actions[m] = addAction(icon, lbl,
               this, [this, m]() { setMode(m); });
    actions[m]->setShortcut(QKeySequence(k));
    actions[m]->setCheckable(true);
    actions[m]->setToolTip(lbl + " (" + QKeySequence(k).toString() +")");
    return actions[m];
  };
  auto addAct = [addAct2](Mode m, QString lbl, Qt::Key k) {
    return addAct2(m, lbl, lbl, k);
  };
  setFloatable(false);
  setMovable(false);
  setIconSize(QSize(24, 24));
  m = Mode::Invalid;
  addAct(Mode::Edit, "Edit", Qt::Key_F1);
  addAct(Mode::PlaceTrace, "Trace", Qt::Key_F2);
  //  addAct(Mode::PlaceComponent, "Component", Qt::Key_F3);
  addAct(Mode::PlaceHole, "Hole", Qt::Key_F3);
  addAct(Mode::PlacePad, "Pad", Qt::Key_F4);
  addAct(Mode::PlaceText, "Text", Qt::Key_F5);
  addAct(Mode::PlaceArc, "Arc", Qt::Key_F6);
  addAct2(Mode::PlacePlane, "Plane", "Filled planes", Qt::Key_F7);
  addAct2(Mode::PickupTrace, "Disconnect", "Pickup trace", Qt::Key_F8);
  addAct2(Mode::PlaceNPHole, "NPHole", "Nonplated hole", Qt::Key_F9);
  //  addAct2(Mode::BoardOutline, "BoardOutline", "Board outline", Qt::Key_F10);
  addAct2(Mode::PNPOrient, "PNPOrient", "Pick ’n’ place component orientation",
          Qt::Key_F10);

  addSeparator();
  a_origin = addAct(Mode::SetIncOrigin, "AbsOrigin", Qt::Key_F11);

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
