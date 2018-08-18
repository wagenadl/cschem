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
  return matchQuality(o) != MatchQuality::None;
}


MatchQuality Nodename::matchQuality(Nodename const &o) const {
  if (comp_ != o.comp_)
    return MatchQuality::None;
  
  if (pin_ == o.pin_) // including the case where both are empty
    return MatchQuality::Perfect;
  
  QSet<QString> theirPins = QSet<QString>::fromList(o.pin_.split("/"));
  MatchQuality mq = MatchQuality::None;
  for (QString const &p: pin_.split("/")) {
    if (theirPins.contains(p) && !p.isEmpty()) {
      if (p[0].isDigit()) {
	if (mq<MatchQuality::Number)
	  mq = MatchQuality::Number;
	else if (mq<MatchQuality::Name)
	  mq = MatchQuality::Name;
      }
    }
  }
  return mq;
}

