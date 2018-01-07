// LibraryPane.cpp

#include "LibraryPane.h"
#include "svg/PartLibrary.h"
#include <algorithm>
#include <QDebug>

class LibraryPaneData {
public:
};

LibraryPane::LibraryPane(QWidget *parent): QListWidget(parent) {
  connect(this, SIGNAL(itemActivated(QListWidgetItem *)),
          this, SLOT(activateHelper(QListWidgetItem *)));
}

LibraryPane::LibraryPane(PartLibrary const *lib, QWidget *parent):
  LibraryPane(parent) {
  if (lib)
    rebuild(lib);
}
 
LibraryPane::~LibraryPane() {
}

void LibraryPane::rebuild(PartLibrary const *lib) {
  clear();
  QStringList parts = lib->partNames();
  std::sort(parts.begin(), parts.end());
  for (QString s: parts) 
    if (s.startsWith("port:") || s.startsWith("part:"))
      addItem(s);
}  

void LibraryPane::activateHelper(QListWidgetItem *item) {
  qDebug() << "activate" << item->text();
  emit activated(item->text());
}
