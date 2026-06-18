// TicToc.h

#ifndef TICTOC_H

#define TICTOC_H

#include <QString>
#include <QElapsedTimer>

class TicToc { // results are milliseconds
public:
  TicToc() { timer.start(); ns=0; }
  float lap() {
    qint64 dt = timer.nsecsElapsed() - ns;
    ns += dt;
    return dt / 1e6f;
  }
  float total() { return timer.nsecsElapsed() / 1e6f; }
private:
  QElapsedTimer timer;
  qint64 ns;  
};

#endif
