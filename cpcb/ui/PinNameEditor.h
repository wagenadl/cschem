// PinNameEditor.h

#ifndef PINNAMEEDITOR_H

#define PINNAMEEDITOR_H

#include <QDialog>

class PinNameEditor: public QDialog {
  Q_OBJECT;
public:
  PinNameEditor(QString group_ref, QString old_pin_ref,
                class Symbol const &sym,
		QWidget *parent=0, int pincount=-1);
  virtual ~PinNameEditor();
  QString pinRef() const;
private:
  void selectionMade(QString);
private:
  QString pin_ref;
};

#endif
