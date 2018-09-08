// SignalAccumulator.h

#ifndef SIGNALACCUMULATOR_H

#define SIGNALACCUMULATOR_H

#include <QTimer>

class SignalAccumulator: public QTimer {
  Q_OBJECT;
public:
  SignalAccumulator(QObject *parent=0);
public slots:
  void activate();
  /* Any number of calls to the activate() slot will result in a single
     activated() signal to be emitted 1 ms after the last call. */
signals:
  void activated();
};

#endif
