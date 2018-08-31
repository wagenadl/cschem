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
  Group const &univ(grpid.isEmpty() ? root : root.object(grpid).asGroup());
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
      //if (obj.asTrace().layer!=Layer::Silk)
      //  qDebug() << "testing" << tr << "vs" << obj.asTrace();
      if (obj.asTrace().touches(tr))
        insertRecursively(nid);
      break;
    case Object::Type::Plane:
      qDebug() << "Test for FP NYI";
      break;
    case Object::Type::Group:
      insertFriendsOfTrace(tr, nid);
    default:
      break;
    }
  }
}

void Builder::insertFriendsOfHole(Hole const &h, NodeID grpid) {
  Group const &univ(grpid.isEmpty() ? root : root.object(grpid).asGroup());
  for (int id: univ.keys()) {
    NodeID nid = grpid.plus(id);
    if (net.contains(nid))
      continue;
    Object const &obj(univ.object(id));
    switch (obj.type()) {
    case Object::Type::Trace:
      if (h.touches(obj.asTrace()))
        insertRecursively(nid);
      break;
    case Object::Type::Plane:
      qDebug() << "Test for FP NYI";
      break;
    default:
      break;
    }
  }      
}

void Builder::insertFriendsOfPad(Pad const &pad, NodeID grpid) {
  Group const &univ(grpid.isEmpty() ? root : root.object(grpid).asGroup());
  for (int id: univ.keys()) {
    NodeID nid = grpid.plus(id);
    if (net.contains(nid))
      continue;
    Object const &obj(univ.object(id));
    switch (obj.type()) {
    case Object::Type::Trace:
      if (pad.touches(obj.asTrace()))
        insertRecursively(nid);
      break;
    case Object::Type::Plane:
      qDebug() << "Test for FP NYI";
      break;
    default:
      break;
    }
  }      
}

void Builder::insertFriendsOfPlane(FilledPlane const &, NodeID) {
  qDebug() << "Test for FP NYI";
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
  if (!somenode.isValid()) {
    for (NodeID const &n: nodes_) {
      somenode = root_.nodeName(n);
      if (somenode.isValid())
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
