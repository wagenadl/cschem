// PinMapDialog.h

#ifndef PINMAPDIALOG_H

#define PINMAPDIALOG_H

#include <QDialog>
#include <QStringList>
#include <QMap>

class PinMapDialog: public QDialog {
  Q_OBJECT;
public:
  PinMapDialog(QWidget *parent=0);
  virtual ~PinMapDialog();
public slots:
  void setReference(QString); // e.g. "A1"
  void setCircuitNames(QStringList const &);
  /* Circuit names can be "K", "+", "3/+". */
  void setPCBNames(QStringList const &);
  /* PCB names are normally "1", "2", "3", but can be "K", "A",
     etc. They are stripped of anything following first "/". */
  void setMap(QMap<QString, QString> const &);
  /* Mapping is PCB name to circuit name. */
  void autoMap();
  void setMapping(QString pcb, QString circ);
public:
  QMap<QString, QString> map() const;
  QString map(QString) const;
  /* Maps a PCB name to a Circuit name. Anything following first "/"
     in PCB name is stripped. */
  static QString stripPCBName(QString);
private:
  class PMDData *d; 
};

#endif
