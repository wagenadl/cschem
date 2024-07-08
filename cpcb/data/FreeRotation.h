// FreeRotation.h

#ifndef FREEROTATION_H

#define FREEROTATION_H

#include <QString>

class FreeRotation {
public:
  FreeRotation();
  explicit FreeRotation(double r); // 0=up, 90=right, 180=down, 270=left
  double radians() const;
  double degrees() const;
  FreeRotation &operator+=(double dr);
  FreeRotation &operator-=(double dr);
  FreeRotation &operator+=(FreeRotation const &dr);
  FreeRotation &operator-=(FreeRotation const &dr);
  FreeRotation operator-() const;
  bool operator==(FreeRotation const &) const;
  bool operator<(FreeRotation const &) const;
  double cos() const;
  double sin() const;
  void flipLeftRight();
  void flipUpDown();
  bool isCardinal() const;
  QString toString() const;
private:
  void normalize();
  double r;
};

QDebug operator<<(QDebug, FreeRotation const &);

#endif
