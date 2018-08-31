// TraceRepair.h

#ifndef TRACEREPAIR_H

#define TRACEREPAIR_H

#include "NodeID.h"
#include "Dim.h"

class TraceRepair {
public:
  TraceRepair(class EData *ed);
  ~TraceRepair();
  void fixTraceIntersections(NodeID id, Dim grid=Dim());
  // id is relative to layout root and must be a trace
  // intersections are snapped to nearest grid point, if nonzero
  void fixPinTouchings(NodeID id);
  // id is relative to layout root and must be a trace
private:
  class RepairData *d;
};

#endif
