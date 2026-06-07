// NetGraph.cpp

#include "NetGraph.h"

#include "Rect.h"
#include "NodeID.h"
#include "Object.h"

#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/connected_components.hpp>
#include <boost/graph/breadth_first_search.hpp>
#include <boost/geometry.hpp>
#include <boost/geometry/index/rtree.hpp>
#include <boost/geometry/index/predicates.hpp>
#include <vector>

namespace bg  = boost::geometry;
namespace bgi = boost::geometry::index;

using RPoint = bg::model::point<long int, 2, bg::cs::cartesian>;
using RBox   = bg::model::box<RPoint>;
using RValue = std::pair<RBox, int>; // box + original index


typedef boost::adjacency_list<
  boost::vecS, 
  boost::vecS, 
  boost::undirectedS> SparseGraph;


struct ConnectionRecorder: public boost::default_bfs_visitor {
  QSet<NodeID> &nodes;
  QVector<NodeID> const &map;
  ConnectionRecorder(QSet<NodeID> &nodes,
                     QVector<NodeID> const &map): nodes(nodes), map(map) {}
  template <typename Vertex, typename GraphType>
  void discover_vertex(Vertex v, const GraphType &) const {
    if (v >=0 && v < map.size())
      nodes << map[v];
    else
      qDebug() << "map does not contain" << v;
  }
};


class NetGraphData {
public:
  QVector<NodeID> nodeids;
  QVector<RBox> bboxes;
  QVector<Object const *> objects;
  QHash<NodeID, int> revmap;
  SparseGraph g;
public:
  void addgroup(NodeID nid, Group const &g) {
    for (int id: g.keys()) {
      NodeID nid1 = nid.plus(id);
      Object const &obj(g.object(id));
      if (obj.type() == Object::Type::Group) {
        addgroup(nid1, obj.asGroup());
      } else if (obj.type() == Object::Type::Hole
                 || obj.type() == Object::Type::Pad
                 || obj.type() == Object::Type::Trace
                 || obj.type() == Object::Type::Plane) {
        revmap[nid1] = nodeids.size();
        Rect r = obj.boundingRect();
        bboxes << RBox(RPoint(r.left.raw(), r.top.raw()), RPoint(r.right().raw(), r.bottom().raw()));
        nodeids << nid1;
        objects << &obj;
      }
    }
  }

        
  NetGraphData(Group const &root) {
    nodeids.reserve(10000);
    bboxes.reserve(10000);
    objects.reserve(10000);
    addgroup(NodeID(), root);
    int V = nodeids.size();

    bgi::rtree<RValue, bgi::quadratic<16>> rtree;
    for (int v=0; v<V; v++) 
      rtree.insert({bboxes[v], v});
    
    g = SparseGraph(V);
    for (int v1=0; v1<V-1; v1++) {
      RBox const &box = bboxes[v1];
      rtree.remove({box, v1});
      std::vector<RValue> hits;      
      rtree.query(bgi::satisfies([&](RValue const &v) { return bg::intersects(v.first, box); }),
                  std::back_inserter(hits));
      for (auto const &rv: hits)
        if (objects[v1]->touches(*objects[rv.second]))
          boost::add_edge(v1, rv.second, g);
    }
  }
};


NetGraph::NetGraph(Group const &root): root(root) {
  d = new NetGraphData(root);
}

NetGraph::~NetGraph() {
  delete d;
}

QSet<NodeID> NetGraph::net(NodeID seed) const {
  QSet<NodeID> res;
  if (!d->revmap.contains(seed)) {
    qDebug() << "Seed not found";
    return res;
  }
  ConnectionRecorder conrec(res, d->nodeids);
  int iseed = d->revmap[seed];
  boost::breadth_first_search(d->g, iseed, boost::visitor(conrec));
  return res;  
}

QList<QSet<NodeID>> NetGraph::allNets() const {
  int V = d->nodeids.size();
  std::vector<int> component(V);
  int K = boost::connected_components(d->g, &component[0]);
  QList<QSet<NodeID>> res(K);
  for (int v = 0; v < V; v++) 
    res[component[v]] << d->nodeids[v];
  return res;
}

Nodename NetGraph::someNodename(QSet<NodeID> const &net, NodeID seed) const {
  Nodename res;
  if (seed.size()) {
    res = root.nodeName(seed);
    if (res.pin() != "")
      return res;
  }
  bool tolerable = res.isValid();
  for (NodeID const &nid: net) {
    Nodename name = root.nodeName(nid);
    if (tolerable) {
      // only take if improves
      if (name.isValid() && name.pin() != "")
        return name;
    } else {
      // take if good, store if ok
      if (name.isValid()) {
        if (name.pin() != "")
          return name;
        res = name;
        tolerable = true;
      }
    }
  }
  return res;
}
