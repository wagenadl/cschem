// Net.h

#ifndef NET_H

#define NET_H

#include "PinID.h"
#include <QSharedData>

class Net {
public:
  Net(class Circuit const &circ, PinID const &seedpin);
  Net(class Circuit const &circ, int seedelt, QString seedpin);
  Net(class Circuit const &circ, int seedcon);
  Net(Net const &);
  Net &operator=(Net const &);
  ~Net();
  QSet<int> connections() const;
  QSet<PinID> pins() const;
private:
  QSharedDataPointer<class NetData> d;
};

#endif
