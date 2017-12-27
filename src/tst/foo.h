// foo.h

#ifndef FOO_H

#define FOO_H

#include <QSharedData>

class foodata;

class foo {
public:
  foo();
  ~foo();
  foo(foo const &);
  foo &operator=(foo const &);
  int &x();
private:
  QSharedDataPointer<foodata> d;
};

#endif
