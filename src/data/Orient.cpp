// Orient.cpp

#include "Orient.h"
#include <QStringList>

QString Orient::toString() const {
  return QString::number(rot) + " " + QString::number(int(flip));
}

Orient Orient::fromString(QString s, bool *ok) {
  Orient p;
  bool ok1 = false;
  QStringList l = s.split(" ");
  if (l.size()==2) {
    p.rot = l[0].toInt(&ok1) & 3;
    if (ok1)
      p.flip = l[1].toInt(&ok1);
  }
  if (ok)
    *ok = ok1;
  return p;
}

QDebug operator<<(QDebug dbg, Orient const &p) {
  dbg << "(" << p.rot << "," << p.flip << ")";
  return dbg;
}

