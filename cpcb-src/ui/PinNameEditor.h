// PinNameEditor.h

#ifndef PINNAMEEDITOR_H

#define PINNAMEEDITOR_H

#include <QDialog>

class PinNameEditor: public QDialog {
public:
  PinNameEditor(QString group_ref, QString old_pin_ref, class Symbol const &sym,
		QWidget *parent=0);
  virtual ~PinNameEditor();
  QString pinRef() const;
private:
  QString pin_ref;
};

#endif
