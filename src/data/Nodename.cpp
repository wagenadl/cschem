// Nodename.cpp

#include "Nodename.h"
#include <QSet>

Nodename::Nodename(QString component, QString pin):
  comp_(component), pin_(pin) {
}

QString Nodename::toString() const {
  if (pin_.isEmpty())
    return comp_;
  else
    return comp_ + ":" + pin_;
}

QString Nodename::humanName() const {
  if (pin_.isEmpty())
    return comp_;
  else
    return "pin " + pin_ + " of " + comp_;
}

bool Nodename::isValid() const {
  return !comp_.isEmpty();
}

bool Nodename::operator==(Nodename const &o) const {
  return comp_==o.comp_ && pin_==o.pin_;
}

bool Nodename::matches(Nodename const &o) const {
  if (comp_ != o.comp_)
    return false;
  else if (pin_ == o.pin_)
    return true;
  if (hasPinName() && o.hasPinName()) 
    return pinName() == o.pinName();
  else if (hasPinNumber() && o.hasPinNumber())
    return pinNumber() == o.pinNumber();
  else
   return false;
}

bool Nodename::hasPinName() const {
  for (QString s: pin_.split("/"))
    if (!s.isEmpty() && !s[0].isDigit())
      return true;
}

bool Nodename::hasPinNumber() const {
  for (QString s: pin_.split("/"))
    if (!s.isEmpty() && s[0].isDigit())
      return true;
}

QString Nodename::pinName() const {
  for (QString s: pin_.split("/"))
    if (!s.isEmpty() && !s[0].isDigit())
      return s;
  return "";
}

int Nodename::pinNumber() const {
  for (QString s: pin_.split("/"))
    if (!s.isEmpty() && s[0].isDigit())
      return s.toInt();
  return 0;
}

