// PCBNet.cpp

#include "PCBNet.h"
#include "Group.h"
#include "Object.h"
#include "Point.h"
#include "LayerPoint.h"

#include <QMultiMap>

class Builder {
public: 
  Builder(Group const &root, NodeID seed);
  QSet<NodeID> const &pcbNet() const { return net; }
  void buildNetLocations(LayerPoint p);
  void addConnections(Group const &root);
  void buildPCBNet(Group const &root, NodeID pfx);
private:
  QMultiMap<LayerPoint, LayerPoint> connections;
  QSet<LayerPoint> in;
  QSet<NodeID> net;
};

Builder::Builder(Group const &root, NodeID seed) {
  if (seed.isEmpty())
    return;
  addConnections(root);
  buildNetLocations(seed.location(root));
  buildPCBNet(root, NodeID());
}

void Builder::buildPCBNet(Group const &root, NodeID pfx) {
  for (int id: root.keys()) {
    Object const &obj(root.object(id));
    if (obj.isTrace()) {
      Trace const &h(obj.asTrace());
      LayerPoint lp1(h.layer, h.p1);
      LayerPoint lp2(h.layer, h.p2);
      if (in.contains(lp1) || in.contains(lp2))
	net << pfx.plus(id);
    } else if (obj.isHole()) {
      Hole const &h(obj.asHole());
      LayerPoint lp1(Layer::Top, h.p);
      LayerPoint lp2(Layer::Bottom, h.p);
      if (in.contains(lp1) || in.contains(lp2))
	net << pfx.plus(id);
    } else if (obj.isPad()) {
      Pad const &h(obj.asPad());
      LayerPoint lp1(h.layer, h.p);
      if (in.contains(lp1))
	net << pfx.plus(id);
    } else if (obj.isGroup()) {
      Group const &h(obj.asGroup());
      buildPCBNet(h, pfx.plus(id));
    }
  }
}

void Builder::buildNetLocations(LayerPoint p1) {
  QList<LayerPoint> border;
  QSet<LayerPoint> seen;

  border << p1;
  
  while (!border.isEmpty()) {
    LayerPoint lp(border.takeLast());
    in << lp;
    auto rng(connections.equal_range(lp));
    for (auto it=rng.first; it!=rng.second; ++it)
      if (!seen.contains(*it))
	border << *it;
    seen << lp;
  }
}

void Builder::addConnections(Group const &root) {
  for (int id: root.keys()) {
    Object const &obj(root.object(id));
    if (obj.isHole()) {
      Hole const &h(obj.asHole());
      LayerPoint lp1(Layer::Top, h.p);
      LayerPoint lp2(Layer::Bottom, h.p);
      connections.insert(lp1, lp2);
      connections.insert(lp2, lp1);
    } else if (obj.isTrace()) {
      Trace const &h(obj.asTrace());
      LayerPoint lp1(h.layer, h.p1);
      LayerPoint lp2(h.layer, h.p2);
      connections.insert(lp1, lp2);
      connections.insert(lp2, lp1);
    }	else if (obj.isGroup()) {
      addConnections(obj.asGroup());
    }
  }
}

PCBNet::PCBNet(): somenode("", "") {
}

PCBNet::PCBNet(Group const &root, NodeID seed): root_(root), seed_(seed),
						somenode("", "") {
  Builder builder(root, seed);
  nodes_ = builder.pcbNet();
  havesomenode = false;
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
