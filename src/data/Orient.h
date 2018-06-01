// Orient.h

#ifndef ORIENT_H

#define ORIENT_H

#include <QDebug>
#include "Dim.h"


struct Orient {
  int rot; // 0..3; 1 turns +ve x into +ve y, i.e., CW.
  bool flip;
public:
  explicit Orient(int rot=0, bool flip=false): rot(rot&3), flip(flip) { }
  QString toString() const;
  static Orient fromString(QString s, bool *ok=0);
};

QDebug operator<<(QDebug, Orient const &);


#endif
