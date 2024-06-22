// PatternHelper.h

#ifndef PATTERNHELPER_H

#define PATTERNHELPER_H

#include "data/Group.h"
#include "data/Object.h"
#include <QSet>

class PatternHelper {
  // helper class to attach reftexts to groups copied in pattern
public:
  PatternHelper(Group &here, QSet<int> const &selection);
  void addItem(int oldid, int newid);
  void apply();
private:
  Group &here;
  QMap<int, int> group2reftext;
  QMap<int, QList<int>> reftext2copies;
  QMap<int, QList<int>> group2copies;
};

#endif
