// Cleanup.h

#ifndef CLEANUP_H

#define CLEANUP_H

#include "NetMismatch.h"
#include "Group.h"
#include "LinkedSchematic.h"

class Cleanup {
  enum class Result {
    Success, // operation succeeded and changed the data
    PartialSuccess, // some things succeeded, but others could not get
    // started because they would cause net errors
    NotStarted, // operation could start because it would cause net errors
    NOP, // nothing had to be done
    CompletedWithErrors, // only returned if FORCE is specified, indicates
    // that the operation went through but that there are now net errors
  };
  enum class Method {
    Safe, // only make any changes if none of them cause net mismatches
    Partial, // make partial changes if the full change set would cause
    // net mismatches. (This may be slow, because NetMismatch has to be
    // called after every single change.)
    Force, // apply all changes, even if net mismatches result
    Ignore, // do not test for net mismatches
  };
public:
  Cleanup(Group const &src, LinkedSchematic const *ls);
  Group const &endProduct() const;
  NetMismatch const &mismatch() const;
  Result snapToGrid(Dim spacing, Method mth=Safe);
  // Snap all items except those with front panel material to the grid
  Result snapTo45(Method mth=Safe);
  // Snap all diagonal-ish segments that are between h. or v. segments to 45 deg.
  Result ensureTouchPoints(Dim marg, Method mth=Safe);
  // Ensure that segments that physically touch do so at instantiated points.
  Result combineSegments(Method mth=Safe);
  // Any colinear segments that meet at a point that is not otherwise relevant
  // will be joined
private:
  Group g;
  LinkedSchematic const *ls;
  NetMismatch nm;
};

#endif
