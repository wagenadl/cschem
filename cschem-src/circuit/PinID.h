// PinID.h

#ifndef PINID_H

#define PINID_H

#include <QPair>
#include <QString>

class PinID: public QPair<int, QString> {
public:
  static constexpr char const *NOPIN = "//";
  explicit PinID(int elt=-1, QString pin=""): QPair<int, QString>(elt, pin) { }
  int element() const { return first; }
  QString pin() const { return second; }
  void setElement(int elt) { first = elt; }
  void setPin(QString pin) { second = pin; }
  bool isValid() const { return first>0 && second!=NOPIN; }
};

#endif
