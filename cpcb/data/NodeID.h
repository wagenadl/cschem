// NodeID.h

#ifndef NODEID_H

#define NODEID_H

#include <vector>
#include "VectorCf.h"
#include "LayerPoint.h"

class NodeID: public std::vector<int> {
public:
  NodeID() {}
  NodeID(std::vector<int>::const_iterator a,
         std::vector<int>::const_iterator b): std::vector<int>(a, b) {}
  NodeID(int node): std::vector<int>{node} {}
  bool operator==(NodeID const &x) const { return vectorEq(*this, x); }
  bool operator<(NodeID const &x) const { return vectorLt(*this, x); }
  LayerPoint location(class Group const &root,
                      Point const &near=Point(), Dim margin=Dim()) const;
  NodeID plus(int x) const { NodeID res(begin(), end());
    res.emplace_back(x); return res; }
  NodeID tail() const { return NodeID(++begin(), end()); }
  NodeID parent() const { return NodeID(begin(), --end()); }
  int last() const { return begin()==end() ? -1 : *rbegin(); }
  int first() const { return begin()==end() ? -1 : *begin(); }
  bool isEmpty() const { return empty(); }
  NodeID &operator<<(int x) { emplace_back(x); return *this; }
  void append(NodeID const &xx) { for (auto x: xx) emplace_back(x); }
  void removeLast() { if (begin()!=end()) erase(--end()); }
  int takeFirst() { if (begin()==end()) return -1; int x = *begin(); erase(begin()); return x;}
  int takeLast() { if (begin()==end()) return -1; int x = *rbegin(); erase(begin()); return x;}
  void push_front(int x) { insert(begin(), x); }
};

inline uint qHash(NodeID const &id) {
  uint hsh = 0;
  for (int bit: id) {
    hsh = 2346781*hsh ^ bit;
  }
  return hsh;
}

#endif
