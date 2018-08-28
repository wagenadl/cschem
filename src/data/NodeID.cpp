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

LayerPoint NodeID::location(Group const &root, bool *okp) const {
  bool ok;
  if (!okp)
    okp = &ok;
  *okp = false;
  if (isEmpty() || !root.contains(*begin()))
    return LayerPoint();
  Object const &obj(root.object(*begin()));
  switch (obj.type()) {
  case Object::Type::Pad:
    *okp = true;
    return LayerPoint(obj.asPad().layer, obj.asPad().p);
  case Object::Type::Hole:
    *okp = true;
    return LayerPoint(Layer::Top, obj.asHole().p);
  case Object::Type::Trace:
    *okp = true;
    return LayerPoint(obj.asTrace().layer, obj.asTrace().p1);
  case Object::Type::Group:
    return tail().location(obj.asGroup(), okp);
  default:
    return LayerPoint();
  }
}

