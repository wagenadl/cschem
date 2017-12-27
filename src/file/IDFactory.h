// IDFactory.h

#ifndef IDFACTORY_H

#define IDFACTORY_H

class IDFactory {
public:
  static IDFactory &instance();
  int newId();
  void reserve(int id);
private:
  IDFactory();
  int max;
};

#endif
