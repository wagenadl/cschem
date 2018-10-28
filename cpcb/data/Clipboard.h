// Clipboard.h

#ifndef CLIPBOARD_H

#define CLIPBOARD_H

#include "Group.h"
#include "Object.h"
#include <QSet>
#include <QList>
#include <QMimeData>

class Clipboard {
  // This is the global clipboard for parts of a PCB layout
public:
  static Clipboard &instance();
  static constexpr char const *dndformat = "application/x-dnd-cpcb-selection";
  static QMimeData *createMimeData(Group const &);
  static Group parseMimeData(QMimeData const *);
public:
  bool isValid() const;
  void store(Group const &root, QSet<int> selection);
  Group retrieve() const;
private:
  Clipboard();
};

#endif
