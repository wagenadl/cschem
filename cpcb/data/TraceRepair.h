// TraceRepair.h

#ifndef TRACEREPAIR_H

#define TRACEREPAIR_H

#include "NodeID.h"
#include "Dim.h"

class TraceRepair {
public:
  TraceRepair(class Group &group);
  ~TraceRepair();

  bool fixTraceIntersections();
  bool fixPinTouchings();
  bool dropDanglingTraces();
  // return true if anything done
private:
  class RepairData *d;
};

#endif
