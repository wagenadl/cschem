// MainWindow.cpp

#include "MainWindow.h"
#include "Modebar.h"
#include "Statusbar.h"

class MWData {
public:
  MWData(MainWindow *mw): mw(mw) {
  }
public:
  MainWindow *mw;
  Modebar *modebar;
  Statusbar *statusbar;
};

MainWindow::MainWindow(): QMainWindow() {
  d = new MWData(this);
  addToolBar(Qt::LeftToolBarArea,
	     d->modebar = new Modebar(this));
  d->statusbar = new Statusbar(this);
  setStatusBar(d->statusbar);
}

MainWindow::~MainWindow() {
  delete d;
}
