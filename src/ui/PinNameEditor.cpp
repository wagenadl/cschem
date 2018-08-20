// PinNameEditor.cpp

#include "PinNameEditor.h"
#include "SignalNameCombo.h"

#include <QHBoxLayout>
#include <QLabel>

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
  QComboBox *cbox = new SignalNameCombo(sym);
  lay->addWidget(cbox);
  setLayout(lay);

  connect(cbox,
	  static_cast<void(QComboBox::*)(const QString &)>
	  (&QComboBox::currentIndexChanged),
	  [this](QString const &s) {
	    if (pin_ref.isEmpty())
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
