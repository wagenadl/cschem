// LibraryPane.cpp

#include "LibraryPane.h"
#include "svg/PartLibrary.h"

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
  for (QString s: lib->partNames())
    addItem(s);
}  

void LibraryPane::activateHelper(QListWidgetItem *item) {
  qDebug() << "activate" << item->text();
  emit activated(item->text());
}
