// SignalNameCombo.cpp

#include "SignalNameCombo.h"

#include "svg/Symbol.h"

SignalNameCombo::SignalNameCombo(Symbol const &sym, QWidget *parent):
  QComboBox(parent) {
  QStringList pins;
  for (QString p: sym.pinNames()) {
    QStringList bits = p.split("/");
    QString best;
    for (QString b: bits) {
      if (best.isEmpty() || (!b.isEmpty() && !b[0].isDigit()))
	best = b;
    }
    pins << best;
  }
  int sc = sym.slotCount();
  for (int slot: sym.containerSlots()) 
    for (QString name: sym.containedPins(slot).keys())
      if (sc>1)
	pins << QString("%1:%s").arg(slot).arg(name);
      else
	pins << name;
  for (QString p: pins)
    addItem(p);
}

SignalNameCombo::~SignalNameCombo() {
}
