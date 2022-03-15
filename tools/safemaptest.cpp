// safemaptest.cpp

#include "SafeMap.h"

int main() {
  SafeMap<QString, int> foo;
  foo["Apple"] = 1;
  foo["Bear"] = 2;
  qDebug() << foo["Cake"];
  qDebug() << foo;
  return 0;
  
}

  

