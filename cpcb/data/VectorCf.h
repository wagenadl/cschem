// VectorCf.h

#ifndef VECTORCF_H

#define VECTORCF_H

template <class A, class B> inline bool vectorEq(A const &a, B const &b) {
  if (a.size() != b.size())
    return false;
  auto ait{a.begin()};
  auto bit{b.begin()};
  while (ait!=a.end() && bit!=b.end()) {
    if (*ait != *bit)
      return false;
    ++ait;
    ++bit;
  }
  return true;
}

template <class A, class B> inline bool vectorLt(A const &a, B const &b) {
  auto ait{a.begin()};
  auto bit{b.begin()};
  while (ait!=a.end() && bit!=b.end()) {
    if (*ait < *bit)
      return true;
    else if (*bit < *ait)
      return false;
    ++ait;
    ++bit;
  }
  if (bit!=b.end())
    return true; // a is shorter
  return false; // b is shorter or a==b
}

#endif
