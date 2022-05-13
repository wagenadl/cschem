// NodeID.cpp

#include "NodeID.h"
#include "Object.h"
#include "Group.h"

LayerPoint NodeID::location(Group const &root,
                            Point const &near, Dim margin) const {
  if (isEmpty() || !root.contains(*begin()))
    return LayerPoint();
  Object const &obj(root.object(*begin()));
  switch (obj.type()) {
  case Object::Type::Pad:
    return LayerPoint(obj.asPad().layer, obj.asPad().p);
  case Object::Type::Hole:
    return LayerPoint(Layer::Top, obj.asHole().p);
  case Object::Type::Trace: {
    Trace const &trc(obj.asTrace());
    if (trc.onP1(near, margin))
      return LayerPoint(trc.layer, trc.p1);
    else if (trc.onP2(near, margin))
      return LayerPoint(trc.layer, trc.p2);
    else
      return LayerPoint(trc.layer, trc.projectionOntoSegment(near));
  }
  case Object::Type::Group:
    return tail().location(obj.asGroup(), near);
  default:
    return LayerPoint();
  }
}

