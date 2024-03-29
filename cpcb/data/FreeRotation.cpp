// FreeRotation.cpp

#include "FreeRotation.h"
#include <math.h>
#include "pi.h"

FreeRotation::FreeRotation() {
  r = 0;
}

FreeRotation::FreeRotation(int r): r(r) {
  normalize();
}

FreeRotation::operator int() const {
  return r;
}

FreeRotation &FreeRotation::operator+=(int dr) {
  r += dr;
  normalize();
  return *this;
}

FreeRotation &FreeRotation::operator-=(int dr) {
  r -= dr;
  normalize();
  return *this;
}

FreeRotation FreeRotation::operator-() const {
  FreeRotation r1;
  r1.r = -r;
  r1.normalize();
  return r1;
}

void FreeRotation::normalize() {
  r %= 360;
  if (r<0)
    r += 360;
}


double FreeRotation::sin() const {
  return ::sin(r*PI/180);
}

double FreeRotation::cos() const {
  return ::cos(r*PI/180);
}

void FreeRotation::flipLeftRight() {
  r = -r;
  normalize();
}

void FreeRotation::flipUpDown() {
  r = 180-r;
  normalize();
}

QString FreeRotation::toString() const {
  return QString::number(r);
}
