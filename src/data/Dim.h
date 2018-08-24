// Dim.h

#ifndef DIM_H

#define DIM_H

#include <QDebug>
#include <QString>
#include <cmath>

struct Dim {
  qint64 d;
  static constexpr qint64 PerUM = 10;
  static constexpr qint64 PerMil = 254;
  static constexpr qint64 PerMM = PerUM*1000;
  static constexpr qint64 PerInch = PerMil*1000;
  static constexpr qint64 Infinity = PerInch*1000*1000*1000;
public:
  Dim(): d(0) { }
  Dim &operator+=(Dim const &x) { d+=x.d; return *this; }
  Dim &operator-=(Dim const &x) { d-=x.d; return *this; }
  Dim &operator*=(double x) { d*=x; return *this; }
  Dim &operator/=(double x) { d/=x; return *this; }
  Dim operator+(Dim const &x) const { return Dim(d+x.d); }
  Dim operator-(Dim const &x) const { return Dim(d-x.d); }
  Dim operator*(double x) const { return Dim(d*x); }
  Dim operator/(double x) const { return Dim(d/x); }
  Dim operator-() const { return Dim (-d); }
  bool operator==(Dim const &x) const { return d==x.d; }
  bool operator<(Dim const &x) const { return d<x.d; }
  bool operator>(Dim const &x) const { return d>x.d; }
  bool operator<=(Dim const &x) const { return d<=x.d; }
  bool operator>=(Dim const &x) const { return d>=x.d; }
  bool operator!=(Dim const &x) const { return d!=x.d; }
  double toMM() const { return d*1.0/PerMM; }
  double toMils() const { return d*1.0/PerMil; }
  double toInch() const { return d*1.0/PerInch; }
  QString toString() const { return QString::number(d); }
  bool isNull() const { return d==0; }
  bool isMetric() const { return !isInch(); }
  bool isInch() const { return toMils()==round(toMils()); } // heuristic
  bool isNegative() const { return d<0; }
  bool isPositive() const { return d>0; }
  Dim roundedTo(Dim o) const {
    if (o.isNull())
      return *this;
    else
      return Dim(((2*(d%o.d) >= o.d) ? (d/o.d+1) : (d/o.d)) * o.d);
  }
  int raw() const { return d; }
public:
  static Dim fromMM(float x) { return Dim(int(std::round(PerMM*x))); }
  static Dim fromMils(float x) { return Dim(int(std::round(PerMil*x))); }
  static Dim fromInch(float x) { return Dim(int(std::round(PerInch*x))); }
  static Dim fromString(QString x, bool *ok=0) { return Dim(x.toInt(ok)); }
  static Dim quadrature(Dim const &a, Dim const &b) {
    return sqrt(a.d*a.d + b.d*b.d); }
  static Dim infinity() { return Dim(Infinity); }
private:
  Dim(qint64 d): d(d) { }
  friend Dim operator*(double, Dim const &);
  friend QDebug operator<<(QDebug d, Dim const &x);
  friend uint qHash(Dim const &d);
};

inline Dim operator*(double a, Dim const &x) {
  return Dim(a*x.d);
}

inline QDebug operator<<(QDebug d, Dim const &x) {
  d << x.toInch();
  return d;
}

inline uint qHash(Dim const &d) { return qHash(d.d); }

#endif
