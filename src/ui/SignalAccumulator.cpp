// SignalAccumulator.cpp

#include "SignalAccumulator.h"

SignalAccumulator::SignalAccumulator(QObject *parent): QTimer(parent) {
  setSingleShot(true);
  setInterval(1);
  connect(this, SIGNAL(timeout()),
          this, SIGNAL(activated()));
}

void SignalAccumulator::activate() {
  start();
}


