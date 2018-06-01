// MainWindow.cpp

#include "MainWindow.h"
#include "Modebar.h"

class MWData {
public:
  MWData(MainWindow *mw): mw(mw) {
  }
public:
  MainWindow *mw;
  Modebar *modebar;
};

MainWindow::MainWindow(): QMainWindow() {
  d = new MWData(this);
  addToolBar(Qt::LeftToolBarArea,
	     d->modebar = new Modebar(this));
}

MainWindow::~MainWindow() {
  delete d;
}
