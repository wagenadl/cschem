// TraceRepair.h

#ifndef TRACEREPAIR_H

#define TRACEREPAIR_H

#include "NodeID.h"
#include "Dim.h"

class TraceRepair {
public:
  struct Result {
    int nmoved_overlap = 0;
    int nsplit_intersect = 0;
    int nmoved_pin = 0;
    int nsplit_pin = 0;
    int ndeleted_overlap = 0;
    int njoined = 0;
    int ndeleted_dangling = 0;
    int ndeleted_dangling_via = 0;
    Dim maxmove = Dim();
  };
public:
  TraceRepair(class Group &group);
  ~TraceRepair();

  bool fixTraceIntersections();
  bool fixPinTouchings();
  bool fixTraceFragments();
  bool dropDanglingTraces();
  // return true if anything done
  Result const &result() { return _result; }
private:
  class RepairData *d;
  Result _result;
};

#endif
