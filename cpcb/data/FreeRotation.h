// FreeRotation.h

#ifndef FREEROTATION_H

#define FREEROTATION_H

#include <QString>

class FreeRotation {
public:
  FreeRotation();
  explicit FreeRotation(int r); // 0=up, 90=right, 180=down, 270=left
  operator int() const;
  FreeRotation &operator+=(int);
  FreeRotation &operator-=(int);
  FreeRotation operator-() const;
  double cos() const;
  double sin() const;
  void flipLeftRight();
  void flipUpDown();
  QString toString() const;
private:
  void normalize();
  int r;
};

#endif
