// MainWindow.cpp

#include "MainWindow.h"
#include <QGraphicsView>
#include "Scene.h"
#include "file/Schem.h"
#include "svg/PartLibrary.h"
#include "file/FileIO.h"

class MWData {
public:
  MWData():
    lib(0), view(0) {
  }
public:
  PartLibrary const *lib;
  QGraphicsView *view;
  QSharedPointer<Scene> scene;
  Schem schem;
};
    
MainWindow::MainWindow(PartLibrary const *lib): d(new MWData()) {
  d->lib = lib ? lib : PartLibrary::defaultLibrary();
  d->view = new QGraphicsView(this);
  d->view->setInteractive(true);
  d->view->scale(2, 2);
  d->view->setMouseTracking(true);
  d->view->setDragMode(RubberBandDrag);
  setCentralWidget(d->view);
  create();
}

MainWindow::~MainWindow() {
  delete d;
}

void MainWindow::create() {
  d->view->setScene(0);
  d->scene = new Scene(lib);
  d->view->setScene(d->scene);
  setWindowTitle("QSchem: Untitled");
}

void MainWindow::load(QString fn) {
  create();
  d->schem = FileIO::loadSchematic(fn);
  d->scene->setCircuit(d->schem.circuit());
  setWindowTitle("QSchem: " + fn);
}

void MainWindow::saveAs(QString fn) {
  d->schem->circuit() = d->scene->circuit();
  d->schem->circuit().renumber();
  FileIO::saveSchematic(fn, *d->schem);
  setWindowTitle("QSchem: " + fn);
}

void MainWindow::zoomIn() {
  d->view->scale(2, 2);
}

void MainWindow::zoomOut() {
  d->view->scale(.5, .5);
}

