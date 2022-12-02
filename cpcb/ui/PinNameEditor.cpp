// PinNameEditor.cpp

#include "PinNameEditor.h"
#include "SignalNameCombo.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QToolButton>
#include <QAction>
#include <QDebug>

PinNameEditor::PinNameEditor(QString group_ref, QString old_pin_ref,
			     Symbol const &sym,
			     QWidget *parent,
                             int pincount): QDialog(parent) {
  pin_ref = "";
  QStringList bits = old_pin_ref.split("/");
  for (QString b: bits)
    if (b.toInt()>0)
      pin_ref = b;
 
  QHBoxLayout *lay = new QHBoxLayout;
  QLabel *grp = new QLabel(group_ref + " pin " + pin_ref);
  lay->addWidget(grp);
  auto *cbox = new SignalNameCombo(sym, this, pincount);
  cbox->setCurrent(pin_ref);
  lay->addWidget(cbox);
  auto *ok = new QToolButton();
  ok->setDefaultAction(new QAction("OK", this));
  lay->addWidget(ok);
  setLayout(lay);

  connect(cbox,
	  QOverload<int>::of(&QComboBox::currentIndexChanged),
	  [this, cbox]() {
	    selectionMade(cbox->currentData().toString());
          });
  connect(ok, &QAbstractButton::clicked,
          [this, cbox] {
	    selectionMade(cbox->currentData().toString());
          });
}

PinNameEditor::~PinNameEditor() {
}

QString PinNameEditor::pinRef() const {
  return pin_ref;
}

void PinNameEditor::selectionMade(QString s) {
  qDebug() << "pinname1" << pin_ref << s;
  QStringList oldbits = pin_ref.split("/");
  QString oldpin = oldbits.first();
  QStringList newbits = s.split("/");
  QString newname = newbits.last();
  if (pin_ref.isEmpty())
    pin_ref = s;
  else if (s.isEmpty())
    pin_ref = oldpin;
  else
    pin_ref = oldpin + "/" + newname;
  accept();
}
