// MoveConstraints.h

#ifndef MOVECONSTRAINTS_H

#define MOVECONSTRAINTS_H

class MoveConstraints {
  /* A set of constraining function for points being moved */
public:
  MoveConstraints();
  MoveConstraints(Group const &root,
		  QSet<int> const &selection,
		  QMap<Layer, QSet<Point>> const &selpts,
		  QMap<Layer, QSet<Point>> const &purepts,
		  QMap<Layer, QSet<Point>> const &stuckpts);
  void setMovingDelta(Point const &delta);
  Point apply(Layer layer, Point const &origpt);
  /* The rules are:
     - Objects in SELECTION are going to move by DELTA no matter what.
     - Traces not in selection are subject to deformation if one of their ends
       is contained in PUREPTS or if it is contained in SELPTS but not in
       STUCKPTS.
     - We will never allow traces to become disconnected from their partners,
       so APPLY will always honor the requests for endpoints in SELPTS. It is
       only endpoints in PUREPTS that may be mucked with.
     - If only one trace has an end point in a given PUREPT, we will simply
       enforce 45-degrees constraints.
     - If two traces have end points in a given PUREPT, we will enforce
       constraints on both, choosing the solution that is closest to the
       request from MOVINGDELTA.
     - If more than two traces have end points in a given PUREPT, we first
       figure out which of them already obey the constraint. Those are
       prioritized. Beyond that, priority is in reverse order of addition
       to the circuit. We then find a solution that works for the first two
       traces.
   */
private:
  class MCData *d;
};

#endif
