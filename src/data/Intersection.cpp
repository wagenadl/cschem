// Intersection.cpp

#include "Intersection.h"
#include "Object.h"

Intersection::Intersection(Group const &group, Trace const &trace, bool ae):
  group(group), trace(trace), allowends(ae) {
}

Intersection::Result Intersection::touchingPin() const {
  for (int id: group.keys()) {
    Object const &obj(group.object(id));
    switch (obj.type()) {
    case Object::Type::Group: {
      Result res = Intersection(obj.asGroup(), trace, allowends).touchingPin();
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

Intersection::Result Intersection::touchingTrace() const {
  for (int id: group.keys()) {
    Object const &obj(group.object(id));
    switch (obj.type()) {
    case Object::Type::Group: {
      Result res = Intersection(obj.asGroup(), trace).touchingTrace();
      if (!res.node.isEmpty()) {
	res.node.push_front(id);
	return res;
      }
    } break;
    case Object::Type::Trace: {
      Point p;
      if (obj.asTrace().touches(trace, &p)) {
	Result res;
	res.node << id;
	res.point = p;
	return res;
      }
    } break;
    default:
      break;
    }
  }
  return Result();
}
