#include "foo.h"
#include <QList>

int main() {
  QList<foo> f;
  foo g;
  f.append(g);
  f[0].x() = 4;
  return g.x();
}

  
