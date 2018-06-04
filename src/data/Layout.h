// Layout.h

#ifndef LAYOUT_H

#define LAYOUT_H

#include "Board.h"
#include "Group.h"
#include <QSharedData>
#include <QXmlStreamReader>
#include <QDebug>

class Layout {
public:
  Layout();
  ~Layout();
  Layout(Layout const &);
  Layout &operator=(Layout const &);
public:
  Group const &root() const;
  Group &root();
  Board const &board() const;
  Board &board();
  Group const &group(QList<int> const &path) const; // following breadcrumbs
  // GROUP returns an empty group if breadcrumbs don't lead to a group.
  Group &group(QList<int> const &path);
  // Caution: Do NOT change the empty group that may be returned.
private:
  QSharedDataPointer<class LData> d;
};

QDebug operator<<(QDebug, Layout const &);
QXmlStreamWriter &operator<<(QXmlStreamWriter &, Layout const &);
QXmlStreamReader &operator>>(QXmlStreamReader &, Layout &);

#endif
