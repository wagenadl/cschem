// PinNameEditor.cpp

#include "PinNameEditor.h"
#include "SignalNameCombo.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QToolButton>
#include <QAction>

PinNameEditor::PinNameEditor(QString group_ref, QString old_pin_ref,
			     Symbol const &sym,
			     QWidget *parent): QDialog(parent) {
  pin_ref = "";
  QStringList bits = old_pin_ref.split("/");
  for (QString b: bits)
    if (b.toInt()>0)
      pin_ref = b;
 
  QHBoxLayout *lay = new QHBoxLayout;
  QLabel *grp = new QLabel(group_ref + " pin " + pin_ref);
  lay->addWidget(grp);
  auto *cbox = new SignalNameCombo(sym);
  cbox->setCurrent(pin_ref);
  lay->addWidget(cbox);
  auto *ok = new QToolButton();
  ok->setDefaultAction(new QAction("OK", this));
  lay->addWidget(ok);
  setLayout(lay);

  connect(cbox,
	  QOverload<int>::of(&QComboBox::currentIndexChanged),
	  [this, cbox]() {
	    QString s = cbox->currentData().toString();
	    if (pin_ref.isEmpty() || s.contains("/"))
	      pin_ref = s;
	    else
	      pin_ref += "/" + s;
	    accept(); });
  connect(ok, &QAbstractButton::clicked,
          [this, cbox] {
	    QString s = cbox->currentData().toString();
	    if (pin_ref.isEmpty() || s.contains("/"))
	      pin_ref = s;
	    else
	      pin_ref += "/" + s;
	    accept(); });
}

PinNameEditor::~PinNameEditor() {
}

QString PinNameEditor::pinRef() const {
  return pin_ref;
}
