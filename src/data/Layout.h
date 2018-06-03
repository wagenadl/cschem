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
  Board const &board() const;
  Group const &root() const;
  Board &board();
  Group &root();
private:
  QSharedDataPointer<class LData> d;
};

QDebug operator<<(QDebug, Layout const &);
QXmlStreamWriter &operator<<(QXmlStreamWriter &, Layout const &);
QXmlStreamReader &operator>>(QXmlStreamReader &, Layout &);

#endif
