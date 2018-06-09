// Orient.h

#ifndef ORIENT_H

#define ORIENT_H

#include <QDebug>
#include "Dim.h"


struct Orient {
  int rot; // 0=up, 1=right, 2=down, 3=left
  bool flip;
public:
  explicit Orient(int rot=0, bool flip=false): rot(rot&3), flip(flip) { }
  QString toString() const;
  static Orient fromString(QString s, bool *ok=0);
};

QDebug operator<<(QDebug, Orient const &);


#endif
