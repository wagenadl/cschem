// MainWindow.cpp

#include "MainWindow.h"
#include <QGraphicsView>
#include "Scene.h"
#include "file/Schem.h"
#include "svg/PartLibrary.h"
#include "file/FileIO.h"
#include <QMenuBar>
#include <QApplication>
#include <QStatusBar>
#include <QDir>
#include <QFileDialog>

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
  QString filename;
  static QString lastdir;
};

QString MWData::lastdir;
    
MainWindow::MainWindow(PartLibrary const *lib): d(new MWData()) {
  d->lib = lib ? lib : PartLibrary::defaultLibrary();
  createView();
  createActions();
  create();
}

void MainWindow::createView() {
  d->view = new QGraphicsView(this);
  d->view->setInteractive(true);
  d->view->scale(2, 2);
  d->view->setMouseTracking(true);
  d->view->setDragMode(QGraphicsView::RubberBandDrag);
  setCentralWidget(d->view);
}

void MainWindow::createActions() {
  QMenu *menu = 0;
  QAction *act = 0;

  menu = menuBar()->addMenu(tr("&File"));

  act = new QAction(tr("&New"), this);
  act->setShortcuts(QKeySequence::New);
  act->setStatusTip(tr("Create a new file"));
  connect(act, &QAction::triggered, this, &MainWindow::newAction);
  menu->addAction(act);

  act = new QAction(tr("&Open…"), this);
  act->setShortcuts(QKeySequence::Open);
  act->setStatusTip(tr("Open an existing file"));
  connect(act, &QAction::triggered, this, &MainWindow::openAction);
  menu->addAction(act);

  act = new QAction(tr("&Save"), this);
  act->setShortcuts(QKeySequence::Save);
  act->setStatusTip(tr("Save file"));
  connect(act, &QAction::triggered, this, &MainWindow::saveAction);
  menu->addAction(act);

  act = new QAction(tr("Save &as…"), this);
  act->setShortcuts(QKeySequence::SaveAs);
  act->setStatusTip(tr("Save file with a new name"));
  connect(act, &QAction::triggered, this, &MainWindow::saveAsAction);
  menu->addAction(act);

  act = new QAction(tr("&Quit"), this);
  act->setShortcuts(QKeySequence::Quit);
  act->setStatusTip(tr("Quit the program"));
  connect(act, &QAction::triggered, this, &MainWindow::quitAction);
  menu->addAction(act);

  menu = menuBar()->addMenu(tr("&Edit"));

  
  menu = menuBar()->addMenu(tr("&View"));

  act = new QAction(tr("&Zoom in"), this);
  act->setShortcuts(QList<QKeySequence>() << QKeySequence(QKeySequence::ZoomIn)
		    << QKeySequence(Qt::CTRL + Qt::Key_Equal));
  act->setStatusTip(tr("Zoom in"));
  connect(act, &QAction::triggered, this, &MainWindow::zoomIn);
  menu->addAction(act);

  act = new QAction(tr("&Zoom out"), this);
  act->setShortcuts(QKeySequence::ZoomOut);
  act->setStatusTip(tr("Zoom out"));
  connect(act, &QAction::triggered, this, &MainWindow::zoomOut);
  menu->addAction(act);

  menuBar()->addSeparator();
  menu = menuBar()->addMenu(tr("&Help"));

  act = new QAction(tr("&About"), this);
  act->setStatusTip(tr("Information about this program"));
  connect(act, &QAction::triggered, this, &MainWindow::aboutAction);
  menu->addAction(act);
}

void MainWindow::createStatusBar() {
  statusBar()->showMessage(tr("Ready"));
}

MainWindow::~MainWindow() {
  delete d;
}

void MainWindow::newAction() {
  auto *mw = d->schem.isEmpty() ? this : new MainWindow(d->lib);
  mw->create();
  mw->show();
}

void MainWindow::openAction() {
  if (d->lastdir.isEmpty())
    d->lastdir = QDir::home().absoluteFilePath("Desktop");
  QString fn = QFileDialog::getOpenFileName(0,
					    "Open schematic file",
					    d->lastdir,
					    tr("Schematics (*.schem)"));
  if (!fn.isEmpty()) {
    auto *mw = d->schem.isEmpty() ? this : new MainWindow(d->lib);
    mw->load(fn);
    mw->show();
  }
}

void MainWindow::saveAction() {
  if (d->filename.isEmpty())
    saveAsAction();
  else
    saveAs(d->filename);
}

void MainWindow::saveAsAction() {
  if (d->lastdir.isEmpty())
    d->lastdir = QDir::home().absoluteFilePath("Desktop");
  QString fn = QFileDialog::getSaveFileName(0, tr("Save schematic as…"),
					    d->lastdir,
					    tr("Schematics (*.schem)"));
  if (fn.isEmpty())
    return;
  
  if (!fn.endsWith(".schem"))
    fn += ".schem";
  saveAs(fn);
}

void MainWindow::quitAction() {
  QApplication::quit();
}



void MainWindow::create() {
  d->view->setScene(0);
  d->scene = QSharedPointer<Scene>(new Scene(d->lib));
  d->view->setScene(d->scene.data());
  setWindowTitle("QSchem");
  d->filename = "";
}

void MainWindow::load(QString fn) {
  create();
  d->schem = FileIO::loadSchematic(fn);
  d->scene->setCircuit(d->schem.circuit());
  setWindowTitle(fn);
  d->filename = fn;
}

void MainWindow::saveAs(QString fn) {
  d->schem.circuit() = d->scene->circuit();
  d->schem.circuit().renumber();
  FileIO::saveSchematic(fn, d->schem);
  setWindowTitle(fn);
  d->filename = fn;
}

void MainWindow::markChanged() {
  setWindowTitle(d->filename + " *");
}

void MainWindow::zoomIn() {
  d->view->scale(2, 2);
}

void MainWindow::zoomOut() {
  d->view->scale(.5, .5);
}

void MainWindow::setStatusMessage(QString msg) {
  statusBar()->showMessage(msg);
}

void MainWindow::aboutAction() {
}
