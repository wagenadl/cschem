// PCBNet2.cpp

#include "PCBNet.h"
#include "Object.h"

// Alternate implementation of PCBNet

class Builder {
public: 
  Builder(Group const &root): root(root) { }
  void insertRecursively(NodeID);
  void insertFriendsOfTrace(Trace const &t, NodeID groupid);
  void insertFriendsOfHole(Hole const &h, NodeID groupid);
  void insertFriendsOfPad(Pad const &p, NodeID groupid);
  void insertFriendsOfPlane(FilledPlane const &fp, NodeID groupid);
public:
  Group const &root;
  QSet<NodeID> net;
};

void Builder::insertRecursively(NodeID nid) {
  //  qDebug() << "insertrecursively" << nid;
  if (net.contains(nid))
    return;
  net << nid;
  // now find everything that thouches nid
  Object const &obj(root.object(nid));
  //  qDebug() << "   that is" << obj;
  switch (obj.type()) {
  case Object::Type::Trace: 
    insertFriendsOfTrace(obj.asTrace(), NodeID());
    break;
  case Object::Type::Hole:
    insertFriendsOfHole(obj.asHole(), NodeID());
    break;
  case Object::Type::Pad:
    insertFriendsOfPad(obj.asPad(), NodeID());
    break;
  case Object::Type::Plane:
    insertFriendsOfPlane(obj.asPlane(), NodeID());
    break;
  default:
    break;
  }
}

void Builder::insertFriendsOfTrace(Trace const &tr, NodeID grpid) {
  bool toplevel = grpid.isEmpty();
  Group const &univ(toplevel ? root : root.object(grpid).asGroup());
  bool discountwire = univ.attributes[Group::Attribute::Footprint]
    .contains("trace");
  for (int id: univ.keys()) {
    NodeID nid = grpid.plus(id);
    if (net.contains(nid))
      continue;
    Object const &obj(univ.object(id));
    switch (obj.type()) {
    case Object::Type::Pad:
      if (obj.asPad().touches(tr))
        insertRecursively(nid);
      break;
    case Object::Type::Hole:
      if (obj.asHole().touches(tr))
        insertRecursively(nid);
      break;
    case Object::Type::Trace:
      if (!discountwire && obj.asTrace().touches(tr))
        insertRecursively(nid);
      break;
    case Object::Type::Plane:
      // traces do not touch planes
      break;
    case Object::Type::Group:
      insertFriendsOfTrace(tr, nid);
      break;
    default:
      break;
    }
  }
}

void Builder::insertFriendsOfHole(Hole const &h, NodeID grpid) {
  bool toplevel = grpid.isEmpty();
  Group const &univ(toplevel ? root : root.object(grpid).asGroup());
  bool discountwire = univ.attributes[Group::Attribute::Footprint]
    .contains("trace");
  for (int id: univ.keys()) {
    NodeID nid = grpid.plus(id);
    if (net.contains(nid))
      continue;
    Object const &obj(univ.object(id));
    switch (obj.type()) {
    case Object::Type::Trace:
      if (!discountwire && h.touches(obj.asTrace())) // ignore traces inside groups
        insertRecursively(nid);
      break;
    case Object::Type::Hole:
      if (h.touches(obj.asHole()))
        insertRecursively(nid);
      break;
    case Object::Type::Pad:
      if (h.touches(obj.asPad()))
        insertRecursively(nid);
      break;
    case Object::Type::Plane:
      if (h.touches(obj.asPlane()))
        insertRecursively(nid);
      break;
    case Object::Type::Group:
      insertFriendsOfHole(h, nid);
      break;
    default:
      break;
    }
  }      
}

void Builder::insertFriendsOfPad(Pad const &pad, NodeID grpid) {
  bool toplevel = grpid.isEmpty();
  Group const &univ(toplevel ? root : root.object(grpid).asGroup());
  bool discountwire = univ.attributes[Group::Attribute::Footprint]
    .contains("trace");
  for (int id: univ.keys()) {
    NodeID nid = grpid.plus(id);
    if (net.contains(nid))
      continue;
    Object const &obj(univ.object(id));
    switch (obj.type()) {
    case Object::Type::Hole:
      if (obj.asHole().touches(pad))
        insertRecursively(nid);
      break;
    case Object::Type::Pad:
      if (obj.asPad().touches(pad))
        insertRecursively(nid);
      break;
    case Object::Type::Trace:
      if (!discountwire && pad.touches(obj.asTrace())) // ignore traces inside group
        insertRecursively(nid);
      break;
    case Object::Type::Plane:
      if (pad.touches(obj.asPlane()))
        insertRecursively(nid);
      break;
    case Object::Type::Group:
      insertFriendsOfPad(pad, nid);
      break;
    default:
      break;
    }
  }      
}

void Builder::insertFriendsOfPlane(FilledPlane const &fp, NodeID grpid) {
  Group const &univ(grpid.isEmpty() ? root : root.object(grpid).asGroup());
  for (int id: univ.keys()) {
    NodeID nid = grpid.plus(id);
    if (net.contains(nid))
      continue;
    Object const &obj(univ.object(id));
    switch (obj.type()) {
    case Object::Type::Pad:
      if (obj.asPad().touches(fp))
        insertRecursively(nid);
      break;
    case Object::Type::Hole:
      if (obj.asHole().touches(fp))
        insertRecursively(nid);
      break;
    case Object::Type::Plane:
      if (obj.asPlane().touches(fp))
        insertRecursively(nid);
      break;
    case Object::Type::Group:
      insertFriendsOfPlane(fp, nid);
      break;
    default:
      break;
    }
  }
}


PCBNet::PCBNet(): somenode("", "") {
  havesomenode = false;
}

PCBNet::PCBNet(Group const &root, NodeID seed): root_(root), seed_(seed),
						somenode("", "") {
  // qDebug() << "PCBNet from" << seed;
  havesomenode = false;
  // flood fill using things that touch seed
  Builder bld(root);
  bld.insertRecursively(seed);
  nodes_ = bld.net;
  // report();
}

Nodename PCBNet::someNode() const {
  if (havesomenode)
    return somenode;

  somenode = root_.nodeName(seed_);
  if (!somenode.isValid() || somenode.pin()=="") {
    for (NodeID const &n: nodes_) {
      somenode = root_.nodeName(n);
      if (somenode.isValid() && somenode.pin()!="")
	break;
    }
  }

  havesomenode = true;
  return somenode;
}
  
void PCBNet::report() const {
  QStringList mine;
  for (NodeID n: nodes_)
    mine << root_.nodeName(n).toString();
  qDebug() << "PCBNet" << mine
	   << "from" << root_.nodeName(seed_).toString()
	   << "rep" << someNode().toString();
}
