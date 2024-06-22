// PatternHelper.cpp

#include "PatternHelper.h"

PatternHelper::PatternHelper(Group &here, QSet<int> const &selection):
  here(here) {
  for (int id: selection) {
    if (here.object(id).isGroup()) {
      group2reftext[id] = here.object(id).asGroup().refTextId();
      group2copies[id] = QList<int>();
      reftext2copies[id] = QList<int>();
    }
  }
}

void PatternHelper::addItem(int oldid, int newid) {
  if (group2copies.contains(oldid)) 
    group2copies[oldid] << newid;
  else if (reftext2copies.contains(oldid))
    reftext2copies[oldid] << newid;
}

void PatternHelper::apply() {
  for (int grpid: group2reftext.keys()) {
    int K = group2copies[grpid].size();
    int refid = group2reftext[grpid];
    for (int k=0; k<K; k++) {
      int newgrp = group2copies[grpid][k];
      int newtxt = reftext2copies[refid][k];
      here.object(newgrp).asGroup().setRefTextId(newtxt);
    }
  }
}
