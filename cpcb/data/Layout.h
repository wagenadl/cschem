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
  bool isValid() const;
  void invalidate();
private:
  QSharedDataPointer<class LData> d;
};

QDebug operator<<(QDebug, Layout const &);
QXmlStreamWriter &operator<<(QXmlStreamWriter &, Layout const &);
QXmlStreamReader &operator>>(QXmlStreamReader &, Layout &);

#endif
