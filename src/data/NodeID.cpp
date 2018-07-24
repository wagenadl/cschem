// NodeID.cpp

#include "NodeID.h"
#include "Object.h"
#include "Group.h"

Object const &NodeID::deref(Group const &root) const {
  static Object nul;
  if (isEmpty() || !root.contains(*begin()))
    return nul;
  Object const &obj(root.object(*begin()));
  if (size()==1)
    return obj;
  if (!obj.isGroup())
    return nul;
  else
    return tail().deref(obj.asGroup());
}

LayerPoint NodeID::location(Group const &root, Point ori) const {
  ori += root.origin;
  if (isEmpty() || !root.contains(*begin()))
    return LayerPoint();
  Object const &obj(root.object(*begin()));
  switch (obj.type()) {
  case Object::Type::Pad:
    return LayerPoint(obj.asPad().layer, obj.asPad().p + ori);
  case Object::Type::Hole:
    return LayerPoint(Layer::Top, obj.asHole().p + ori);
  case Object::Type::Trace:
    return LayerPoint(obj.asTrace().layer, obj.asTrace().p1 + ori);
  case Object::Type::Group:
    return tail().location(obj.asGroup(), ori);
  default:
    return LayerPoint();
  }
}

