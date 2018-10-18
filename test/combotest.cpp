// combotest.cpp

#include <QComboBox>
#include <QApplication>
#include <QDebug>
#include "../cpcb/ui/DimSpinner.h"

int main(int argc, char **argv) {
  QApplication ap(argc, argv);
  QComboBox box;
  DimSpinner *sp = new DimSpinner;
  sp->setNoValueText("off");
  sp->setMode(Expression::Mode::Explicit);
  sp->hideTrailingZeros();
  box.setEditable(true);
  box.setInsertPolicy(QComboBox::InsertAtBottom);
  box.addItem("off");
  box.addItem("1”");
  box.addItem("1 mm");
  box.setLineEdit(sp);
  box.setMaxCount(5);
  QObject::connect(&box, QOverload<QString const &>::of(&QComboBox::activated),
		   [&box, sp](QString s) {
		     qDebug() << "activated" << s;
		     sp->parseValue();
		     qDebug() << "sp" << sp->isValid()
			      << sp->hasValue() << sp->value();
		     if (box.currentIndex()==4) {
		       if (sp->isValid())
			 box.removeItem(3);
		       else
			 box.removeItem(4);
		       sp->parseValue();
		     }
		   });
  box.show();
  return ap.exec();
}


