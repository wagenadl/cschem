// SignalNameCombo.cpp

#include "SignalNameCombo.h"

#include "svg/Symbol.h"
#include <QDebug>

SignalNameCombo::SignalNameCombo(Symbol const &sym, QWidget *parent):
  QComboBox(parent) {
  QStringList pins;
  QMap<QString, QString> data;
  for (QString p: sym.pinNames()) {
    QStringList bits = p.split("/");
    QString best;
    for (QString b: bits) {
      if (best.isEmpty() || (!b.isEmpty() && b.toInt()<=0))
	best = b;
    }
    data[best] = p;
    pins << best;
  }
  int sc = sym.slotCount();
  for (int slot: sym.containerSlots()) {
    for (QString name: sym.containedPins(slot).keys()) {
      QString best = (sc>1) ? QString("%1.%2").arg(slot).arg(name) : name;
      data[best] = sym.containedPins(slot)[name] + "/" + best;
      pins << best;
    }
  }
  for (QString p: pins) {
    QString nice = p;
    nice.replace("-", "−");
    addItem(nice, QVariant(data[p]));
  }
}

void SignalNameCombo::setCurrent(QString s) {
  for (int i=0; i<count(); ++i) {
    for (QString bit: itemData(i).toString().split("/")) {
      qDebug() << "setcurrent" << s << "vs" << bit;
      if (bit==s) {
	qDebug() << "setcurrent: gotcha";
	setCurrentIndex(i);
	return;
      }
    }
  }
}

SignalNameCombo::~SignalNameCombo() {
}
