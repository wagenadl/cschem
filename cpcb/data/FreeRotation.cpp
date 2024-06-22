// FreeRotation.cpp

#include "FreeRotation.h"
#include <math.h>
#include "pi.h"
#include <QDebug>

FreeRotation::FreeRotation() {
  r = 0;
}

FreeRotation::FreeRotation(double r): r(r) {
  normalize();
}

double FreeRotation::degrees() const {
  return r;
}

bool FreeRotation::isCardinal() const {
  return fabs(fmod(r, 90)) < 1e-6;
}

bool FreeRotation::operator==(FreeRotation const &fr) const {
  return r == fr.r;
}

bool FreeRotation::operator<(FreeRotation const &fr) const {
  return r < fr.r;
}

FreeRotation &FreeRotation::operator+=(FreeRotation const &dr) {
  *this += dr.r;
  return *this;
}

FreeRotation &FreeRotation::operator-=(FreeRotation const &dr) {
  *this -= dr.r;
  return *this;
}

FreeRotation &FreeRotation::operator+=(double dr) {
  r += dr;
  normalize();
  return *this;
}

FreeRotation &FreeRotation::operator-=(double dr) {
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
  r = fmod(r, 360);
  if (r<0)
    r += 360;
}

double FreeRotation::radians() const {
  return r*PI/180;
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

QDebug operator<<(QDebug d, FreeRotation const &fr) {
  d << fr.degrees();
  return d;
}
