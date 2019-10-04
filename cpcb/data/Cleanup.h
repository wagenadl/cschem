// Cleanup.h

#ifndef CLEANUP_H

#define CLEANUP_H

class Cleanup {
  enum class Result {
    Success, // something changed
    Failure, // operation could not go through because it would cause net errors
    NOP, // nothing had to be done
  };
public:
  Cleanup(Group const &src);
  Group const &result() const;
  Result snapToGrid(Dim spacing);
  // Snap all items except those with front panel material to the grid
  Result snapTo45();
  // Snap all diagonal-ish segments that are between h. or v. segments to 45 deg.
  Result ensureTouchPoints(Dim marg);
  // Ensure that segments that physically touch do so at instantiated points.
  Result combineSegments();
  // Any colinear segments that meet at a point that is not otherwise relevant
  // will be joined
private:
  Group g;
};

#endif
