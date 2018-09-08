// Fraction.h

#ifndef FRACTION_H

#define FRACTION_H

#include <QDebug>

struct Fraction {
  qint64 num;
  qint64 denom;
public:
  explicit Fraction(qint64 n=0, quint64 d=1): num(n), denom(d) { }
};

inline QDebug operator<<(QDebug d, Fraction const &f) {
  d << f.num << "/" << f.denom << "~=" << ((f.denom==0) ? 0 : f.num*1./f.denom);
  return d;
}

#endif
