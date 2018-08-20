// Nodename.h

#ifndef NODENAME_H

#define NODENAME_H

#include <QString>
#include <QSet>

class Nodename {
public:
  Nodename(QString component, QString pin);
  QString component() const { return comp_; }
  QString pin() const { return pin_; }
  QString toString() const;
  QString humanName() const;  
  bool isValid() const;
  bool operator==(Nodename const &o) const;
  bool matches(Nodename const &o) const;
  /* Pins may be named "1/K" or "2/Signal". Or they may be called
     simply "+" or "3". The "1/K" and "2/Signal" have both a pin name
     and a pin number. "+" has only a pin name. "3" has only a pin
     number. Nodenames match when (1) their component name matches and (2) one of the following applies: (2a) both pins are empty. (2b) both have a pin name and they match. (2c) at least one does not have a pin name, but they do have pin numbers, and those match. */
  bool hasPinName() const;
  bool hasPinNumber() const;
  QString pinName() const;
  int pinNumber() const;
private:
  QString comp_;
  QString pin_;
};

inline uint qHash(Nodename const &nn) {
  return qHash(QPair<QString, QString>(nn.component(), nn.pin()));
}

#endif
