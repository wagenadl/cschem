// TraceRepair.h

#ifndef TRACEREPAIR_H

#define TRACEREPAIR_H

#include "NodeID.h"
#include "Dim.h"

class TraceRepair {
public:
  TraceRepair(class Group &group);
  ~TraceRepair();
  bool fixTraceIntersections(int id, Dim grid=Dim());
  // id must be a trace in the group
  // intersections are snapped to nearest grid point, if nonzero
  // returns true if anything done
  bool fixAllTraceIntersections(Dim grid=Dim());
  bool fixPinTouchings(int id);
  // id must be a trace in the group
  // returns true if anything done
  bool fixAllPinTouchings();
  bool dropDanglingTraces();
  // returns true if anything done
private:
  class RepairData *d;
};

#endif
