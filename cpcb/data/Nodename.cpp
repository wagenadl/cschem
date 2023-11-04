// Nodename.cpp

#include "Nodename.h"
#include <QSet>
#include <QDebug>


inline QString simplehyphen(QString s) {
  s.replace("−", "-");
  s.replace("–", "-");
  s.replace("‒", "-");
  // replace minus (0x2212), en-dash (0x2013) and fig-dash (0x2012) by hyphen-minus (0x2d)
  return s;
}

Nodename::Nodename(QString component, QString pin):
  comp_(simplehyphen(component)), pin_(simplehyphen(pin)) {
}

Nodename::Nodename() {
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
  bool isnum;
  pin_.toInt(&isnum);
  if (isnum)
    return "pin " + pin_ + " of " + comp_;
  else
    return "pin “" + pin_ + "” of " + comp_;
}

bool Nodename::isValid() const {
  return !comp_.isEmpty() && !comp_.endsWith("?");
}

bool Nodename::operator<(Nodename const &o) const {
  return comp_==o.comp_ ? pin_<o.pin_ : comp_<o.comp_;
}

bool Nodename::operator==(Nodename const &o) const {
  return comp_==o.comp_ && pin_==o.pin_;
}


bool Nodename::matches(Nodename const &o) const {
  if (comp_==o.comp_) {
    if (pin_ == o.pin_)
      return true;
    if (hasPinName() && o.hasPinName()) 
      return pinName() == o.pinName();
    else if (hasPinNumber() && o.hasPinNumber())
      return pinNumber() == o.pinNumber();
    else
      return false;
  } else {
    QString contcomp = comp_;
    QString subcomp = "";
    QString pinname = pinName();
    int idx = contcomp.indexOf(".");
    if (idx>=0) {
      subcomp = contcomp.mid(idx+1);
      contcomp = contcomp.left(idx);
    } else if (hasPinName()) {
      idx = pinname.indexOf(".");
      if (idx>=0) {
        subcomp = pinname.left(idx);
        pinname = pinname.mid(idx+1);
      } else {
        subcomp = "";
      }
    }
    QString ocontcomp = o.comp_;
    QString osubcomp = "";
    QString opinname = o.pinName();
    idx = ocontcomp.indexOf(".");
    if (idx>=0) {
      osubcomp = ocontcomp.mid(idx+1);
      ocontcomp = ocontcomp.left(idx);
    } else if (o.hasPinName()) {
      idx = opinname.indexOf(".");
      if (idx>=0) {
        osubcomp = opinname.left(idx);
        opinname = opinname.mid(idx+1);
      } else {
        osubcomp = "";
      }
    }
    return contcomp==ocontcomp && subcomp==osubcomp && pinname==opinname;
  }
}

bool Nodename::hasPinName() const {
  for (QString s: pin_.split("/"))
    if (!s.isEmpty() && s.toInt()<=0)
      return true;
  return false;
}

bool Nodename::hasPinNumber() const {
  for (QString s: pin_.split("/"))
    if (s.toInt()>0)
      return true;
  return false;
}

QString Nodename::pinName() const {
  for (QString s: pin_.split("/"))
    if (!s.isEmpty() && s.toInt()<=0)
      return s;
  return "";
}

int Nodename::pinNumber() const {
  for (QString s: pin_.split("/")) {
    int k = s.toInt();
    if (k>0)
      return k;
  }
  return 0;
}

QDebug &operator<<(QDebug &dbg, Nodename const &n) {
  dbg << n.toString();
  return dbg;
}

