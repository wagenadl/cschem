// Intersection.cpp

#include "Intersection.h"
#include "Object.h"

Intersection::Intersection(Group const &group, int traceid):
  group(group), trace(group.object(traceid).asTrace()), traceid(traceid) {
}

Intersection::Intersection(Group const &group, Trace const &trace):
  group(group), trace(trace), traceid(-1) {
}

Intersection::Result Intersection::touchingPin(bool allowends) const {
  for (int id: group.keys()) {
    Object const &obj(group.object(id));
    switch (obj.type()) {
    case Object::Type::Group: {
      Result res = Intersection(obj.asGroup(), trace).touchingPin(allowends);
      if (!res.node.isEmpty()) {
	res.node.push_front(id);
	return res;
      }
    } break;
    case Object::Type::Hole:
      if (obj.asHole().touches(trace)) {
	Result res;
	res.point = obj.asHole().p;
	if (allowends || (res.point != trace.p1 && res.point != trace.p2)) {
	  res.node << id;
	  return res;
	}
      }
      break;
    case Object::Type::Pad:
      if (obj.asPad().touches(trace)) {
	Result res;
	res.point = obj.asPad().p;
	if (allowends || (res.point != trace.p1 && res.point != trace.p2)) {
	  res.node << id;
	  return res;
	}
      }
      break;
    default:
      break;
    }
  }
  return Result();
}

Intersection::Result Intersection::touchingTrace(bool allowends) const {
  for (int id: group.keys()) {
    if (id==traceid)
      continue;
    Object const &obj(group.object(id));
    switch (obj.type()) {
    //case Object::Type::Group: {
    //  // should we go inside subgroups at all?
    //  Result res = Intersection(obj.asGroup(), trace).touchingTrace(allowends);
    //  if (!res.node.isEmpty()) {
    //    res.node.push_front(id);
    //    return res;
    //  }
    //} break;
    case Object::Type::Trace: {
      Point p;
      Trace const &tr1(obj.asTrace());
      if (tr1.touches(trace, &p)) {
        if (allowends
            || (p!=tr1.p1 && p!=tr1.p2)
            || (p!=trace.p1 && p!=trace.p2)) {
          Result res;
          res.node << id;
          res.point = p;
          return res;
        }
      }
    } break;
    default:
      break;
    }
  }
  return Result();
}
