// FreeRotation.h

#ifndef FREEROTATION_H

#define FREEROTATION_H

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
private:
  void normalize();
  int r;
};

#endif
