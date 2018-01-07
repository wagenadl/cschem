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
#include <QDebug>
#include "Style.h"
#include <QMessageBox>
#include "LibraryPane.h"
#include "LibView.h"
#include <QDockWidget>
#include "LibView.h"

class MWData {
public:
  MWData():
    view(0), scene(0), libpane(0) {
  }
public:
  PartLibrary lib;
  QGraphicsView *view;
  Scene *scene;
  LibraryPane *libpane;
  LibView *libview;
  Schem schem;
  QString filename;
  static QString lastdir;
};

QString MWData::lastdir;
    
MainWindow::MainWindow(PartLibrary const *lib): d(new MWData()) {
  if (lib)
    d->lib = *lib;
  else
    d->lib = PartLibrary::defaultLibrary();
  createView();
  createDocks();
  createActions();
  create();
}

void MainWindow::createDocks() {
  QDockWidget *dock;
  
  //  d->libpane = new LibraryPane(&d->lib);
  //  connect(d->libpane, SIGNAL(activated(QString)),
  //          this, SLOT(plonk(QString)));
  //  dock = new QDockWidget("Library", this);
  //  dock->setWidget(d->libpane);
  //  addDockWidget(Qt::LeftDockWidgetArea, dock);

  d->libview = new LibView(&d->lib);
  connect(d->libview, SIGNAL(activated(QString)),
          this, SLOT(plonk(QString)));
  dock = new QDockWidget("Library", this);
  dock->setWidget(d->libview);
  addDockWidget(Qt::LeftDockWidgetArea, dock);
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

  act = new QAction(tr("&Copy"), this);
  act->setShortcuts(QKeySequence::Copy);
  act->setStatusTip(tr("Copy selection to clipboard"));
  connect(act, &QAction::triggered, this, &MainWindow::copyAction);
  menu->addAction(act);
  
  act = new QAction(tr("&Cut"), this);
  act->setShortcuts(QKeySequence::Cut);
  act->setStatusTip(tr("Cut selection to clipboard"));
  connect(act, &QAction::triggered, this, &MainWindow::cutAction);
  menu->addAction(act);
  
  act = new QAction(tr("&Paste"), this);
  act->setShortcuts(QKeySequence::Paste);
  act->setStatusTip(tr("Paste clipboard into circuit"));
  connect(act, &QAction::triggered, this, &MainWindow::pasteAction);
  menu->addAction(act);

  act = new QAction(tr("&Undo"), this);
  act->setShortcuts(QKeySequence::Undo);
  act->setStatusTip(tr("Undo"));
  connect(act, &QAction::triggered, this, &MainWindow::undoAction);
  menu->addAction(act);

  act = new QAction(tr("&Redo"), this);
  act->setShortcuts(QKeySequence::Redo);
  act->setStatusTip(tr("Redo"));
  connect(act, &QAction::triggered, this, &MainWindow::redoAction);
  menu->addAction(act);

  act = new QAction(tr("Remove &dangling connections"), this);
  act->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_B));
  act->setStatusTip(tr("Cleanup circuit by removing dangling connections"));
  connect(act, &QAction::triggered, this, &MainWindow::removeDanglingAction);
  menu->addAction(act);

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

  act = new QAction(tr("&About…"), this);
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
  auto *mw = d->schem.isEmpty() ? this : new MainWindow(&d->lib);
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
    auto *mw = d->schem.isEmpty() ? this : new MainWindow(&d->lib);
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
  if (d->scene) {
    d->view->setScene(0);
    delete d->scene;
  }
  d->scene = new Scene(&d->lib);
  d->view->setScene(d->scene);
  setWindowTitle(Style::programName());
  d->filename = "";
}

void MainWindow::load(QString fn) {
  create();
  d->schem = FileIO::loadSchematic(fn);
  for (QString name: d->schem.library().partNames())
    d->lib.insert(d->schem.library().part(name));
  d->scene->setCircuit(d->schem.circuit());
  setWindowTitle(fn);
  d->filename = fn;
}

void MainWindow::saveAs(QString fn) {
  Circuit circ(d->scene->circuit());
  circ.renumber();
  d->schem.setCircuit(circ);
  d->schem.selectivelyUpdateLibrary(d->lib);
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
  QString me = "<b>" + Style::programName() + "</b>";
  QString vsn = Style::versionName();
  QMessageBox::about(0, "About " + me,
                     me + " " + vsn
                     + "<p>" + "(C) 2018 Daniel A. Wagenaar\n"
                     + "<p>" + me + " is a program for electronic circuit layout with high-quality SVG export. More information is available at <a href=\"http://www.danielwagenaar.net/cschem\">www.danielwagenaar.net/cschem</a>.\n"
                     + "<p>" + me + " is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.\n"
                     + "<p>" + me + " is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.\n"
                     + "<p>" + "You should have received a copy of the GNU General Public License along with this program. If not, see <a href=\"http://www.gnu.org/licenses/gpl-3.0.en.html\">www.gnu.org/licenses/gpl-3.0.en.html</a>.");
}

void MainWindow::copyAction() {
  d->scene->copyToClipboard();
}

void MainWindow::cutAction() {
  d->scene->copyToClipboard(true);
}

void MainWindow::pasteAction() {
  d->scene->pasteFromClipboard();
}

void MainWindow::undoAction() {
  d->scene->undo();
}

void MainWindow::redoAction() {
  d->scene->redo();
}

void MainWindow::removeDanglingAction() {
  d->scene->removeDangling();
}

void MainWindow::plonk(QString sym) {
  d->scene->plonk(sym, d->view->mapToScene(QPoint(d->view->width()/2,
                                                  d->view->height()/2)));
}
