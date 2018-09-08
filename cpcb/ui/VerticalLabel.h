// VerticalLabel.h

#ifndef VERTICALLABEL_H
#define VERTICALLABEL_H

/* Taken from
   https://stackoverflow.com/questions/9183050/vertical-qlabel-or-the-equivalent
   Answer by user Mostafa
   (https://stackoverflow.com/users/593387/mostafa), who
   owns the rights to this code.
*/
   
#include <QLabel>

class VerticalLabel: public QLabel {
  Q_OBJECT;
public:
  explicit VerticalLabel(QWidget *parent=0);
  explicit VerticalLabel(const QString &text, QWidget *parent=0);
  void setCCW(bool ccw);
protected:
  void paintEvent(QPaintEvent*);
  QSize sizeHint() const ;
  QSize minimumSizeHint() const;
private:
  bool ccw;
};

#endif // VERTICALLABEL_H
