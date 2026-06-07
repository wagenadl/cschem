// NetGraph.cpp

#include "NetGraph.h"

#include "Rect.h"
#include "NodeID.h"
#include "Object.h"

#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/connected_components.hpp>
#include <boost/graph/breadth_first_search.hpp>



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
  QVector<Rect> bboxes;
  QVector<Object const *> objects;
  QMap<NodeID, int> revmap;
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
        nodeids << nid1;
        bboxes << obj.boundingRect();
        objects << &obj;
      }
    }
  }

  bool connected(int v1, int v2) {
    if (!bboxes[v1].intersects(bboxes[v2]))
      return false;
    return objects[v1]->touches(*objects[v2]);
  }
        
  NetGraphData(Group const &root) {
    addgroup(NodeID(), root);
    int V = nodeids.size();
    g = SparseGraph(V);
    for (int v1=0; v1<V; v1++)
      for (int v2=v1+1; v2<V; v2++)
        if (connected(v1, v2))
          boost::add_edge(v1, v2, g);
  }
};


NetGraph::NetGraph(Group const &root): d(new NetGraphData(root)) {}

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
