// MainWindow.cpp

#include "MainWindow.h"
#include "Modebar.h"
#include "Propertiesbar.h"
#include "Statusbar.h"
#include "Editor.h"
#include "MultiCompView.h"
#include "data/Paths.h"

#include <QMessageBox>
#include <QMenu>
#include <QMenuBar>
#include <QApplication>
#include <QFileDialog>
#include <QDockWidget>

class MWData {
public:
  MWData(MainWindow *mw): mw(mw) {
    editor = 0;
    mcv = 0;
    mcvdock = 0;
  }
  void about();
  void makeToolbars();
  void makeMenus();
  void makeParts();
  void makeEditor();
  void makeConnections();
  void showHideParts();
  void showParts();
  void hideParts();
  void fillBars();
  void openDialog();
  void saveAsDialog();
  void saveImmediately();
  void linkSchematicDialog();
  void insertComponentDialog();
  void saveComponentDialog();
public:
  MainWindow *mw;
  Modebar *modebar;
  Propertiesbar *propbar;
  Statusbar *statusbar;
  MultiCompView *mcv;
  QDockWidget *mcvdock;
  Editor *editor;
  QString pwd;
  QString compwd;
  QString filename;
};

void MWData::showHideParts() {
  if (!mcv)
    makeParts();
  if (mcvdock->isVisible())
    hideParts();
  else
    showParts();
}

void MWData::hideParts() {
  if (mcvdock)
    mcvdock->hide();
}

void MWData::showParts() {
  if (!mcv)
    makeParts();
  mcvdock->show();
  mw->addDockWidget(Qt::LeftDockWidgetArea, mcvdock);
  mcv->setSchem(editor->linkedSchematic());
  mcv->setRoot(editor->pcbLayout().root());
}

void MWData::openDialog() {
  if (pwd.isEmpty())
    pwd = Paths::defaultLocation();
  
  QString fn = QFileDialog::getOpenFileName(0, "Select file to open…",
					    pwd,
					    "PCB layouts (*.cpcb)");
  if (!fn.isEmpty()) {
    auto *w = editor->pcbLayout().root().isEmpty() ? mw : new MainWindow();
    w->d->filename = fn;
    w->d->pwd = QFileInfo(fn).dir().absolutePath();
    w->setWindowTitle(fn);
    w->d->editor->load(fn);
    w->show();
  }
}

void MainWindow::open(QString fn) {
  d->filename = fn;
  d->pwd = QFileInfo(fn).dir().absolutePath();
  setWindowTitle(fn);
  d->editor->load(fn);
}  

void MWData::saveImmediately() {
  if (filename.isEmpty()) {
    saveAsDialog();
  } else {
    if (!editor->save(filename))
      QMessageBox::warning(mw, "cpcb",
			   "Could not save pcb as “"
			   + filename + "”",
			   QMessageBox::Ok);
  }
}

void MWData::saveComponentDialog() {
  QString msg;
  int id = editor->selectedComponent(&msg);
  if (!id) {
    QMessageBox::warning(mw, "Cannot save component", msg);
    return;
  }

  if (compwd.isEmpty()) {
     compwd = Paths::componentRoot();
     QDir::home().mkpath(compwd);
  }
  QString fn = QFileDialog::getSaveFileName(0, "Save component…",
					    compwd,
					    "PCB components (*.svg)");
  if (fn.isEmpty())
    return;

  if (!fn.endsWith(".svg"))
    fn += ".svg";

  compwd = QFileInfo(fn).dir().absolutePath();
  
  editor->saveComponent(id, fn);
}


void MWData::insertComponentDialog() {
  if (compwd.isEmpty()) {
     compwd = Paths::componentRoot();
     QDir::home().mkpath(compwd);
  }
  Point pt = editor->hoverPoint();
  QString fn = QFileDialog::getOpenFileName(0, "Select file to open…",
					    compwd,
					    "PCB components (*.svg)");
  if (fn.isEmpty())
    return;

  compwd = QFileInfo(fn).dir().absolutePath();

  if (!editor->insertComponent(fn, pt)) 
    QMessageBox::warning(mw, "Failed to insert component",
                         "Cannot insert component “" + fn
                         + "”. Could the file be damaged?");
}

void MWData::linkSchematicDialog() {
  if (pwd.isEmpty()) {
     pwd = Paths::defaultLocation();
     QDir::home().mkpath(pwd);
  }
  QString fn = QFileDialog::getOpenFileName(0, "Link schematic…",
					    compwd,
					    "Schematics (*.schem)");
  if (fn.isEmpty())
    return;

  pwd = QFileInfo(fn).dir().absolutePath();

  if (editor->linkSchematic(fn))
    showParts();
  else
    QMessageBox::warning(mw, "Failed to link schematic",
                         "Cannot link schematic “" + fn
                         + "”. Could the file be damaged?");
}


void MWData::saveAsDialog() {
  if (pwd.isEmpty())
    pwd = Paths::defaultLocation();
  
  QString fn = QFileDialog::getSaveFileName(0, "Save as…",
					    pwd,
					    "PCB layouts (*.cpcb)");
  if (!fn.isEmpty()) {
    if (!fn.endsWith(".cpcb"))
      fn += ".cpcb";
    filename = fn;
    pwd = QFileInfo(fn).dir().absolutePath();
    mw->setWindowTitle(fn);
    if (!editor->save(fn))
      QMessageBox::warning(mw, "cpcb",
			   "Could not save pcb as “"
			   + fn + "”",
			   QMessageBox::Ok);
  }
}  

void MWData::about() {
  QString me = "<b>cpcb</b>";
  QString vsn = "0.01";
  QMessageBox::about(0, "About " + me,
		     me + " " + vsn
		     + "<p>" + "(C) 2018 Daniel A. Wagenaar\n"
		     + "<p>" + me + " is a program for printed circuit board  layout. More information is available at <a href=\"http://www.danielwagenaar.net/cschem\">www.danielwagenaar.net/cschem</a>.\n"
		     + "<p>" + me + " is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.\n"
		     + "<p>" + me + " is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.\n"
		     + "<p>" + "You should have received a copy of the GNU General Public License along with this program. If not, see <a href=\"http://www.gnu.org/licenses/gpl-3.0.en.html\">www.gnu.org/licenses/gpl-3.0.en.html</a>.");
}

void MWData::makeParts() {
  mcvdock = new QDockWidget("Parts", mw);
  mcv = new MultiCompView;
  Q_ASSERT(editor);
  QObject::connect(editor, &Editor::componentsChanged,
		   [this]() { mcv->setRoot(editor->pcbLayout().root()); });
  mcvdock->setWidget(mcv);
  showParts();
}

void MWData::makeToolbars() {
  mw->addToolBar(Qt::LeftToolBarArea,
		 modebar = new Modebar(mw));
  mw->addToolBar(Qt::RightToolBarArea,
		 propbar = new Propertiesbar(editor, mw));
  statusbar = new Statusbar(mw);
  mw->setStatusBar(statusbar);
}

void MWData::makeMenus() {
  auto *mb = mw->menuBar();

  auto *file = mb->addMenu("&File");
  file->addAction("&Open…", [this]() { openDialog(); },
		  QKeySequence(Qt::CTRL + Qt::Key_O));
  file->addAction("&Save", [this]() { saveImmediately(); },
		  QKeySequence(Qt::CTRL + Qt::Key_S));		  
  file->addAction("Save &as…", [this]() { saveAsDialog(); },
		  QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::Key_S));
  file->addAction("&Link schematic…", [this]() { linkSchematicDialog(); },
		  QKeySequence(Qt::CTRL + Qt::Key_L));
  file->addAction("&Unlink schematic", [this]() { editor->unlinkSchematic(); },
		  QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::Key_L));
  file->addAction("&Insert component…", [this]() { insertComponentDialog(); },
		  QKeySequence(Qt::CTRL + Qt::Key_I));
  file->addAction("Save &component…", [this]() { saveComponentDialog(); },
                  QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::Key_I));
  file->addAction("&Quit", []() { QApplication::quit(); });

  auto *edit = mb->addMenu("&Edit");
  edit->addAction("&Rotate clockwise", [this]() { editor->rotateCW(); },
                  QKeySequence(Qt::CTRL + Qt::Key_R));
  edit->addAction("Rotate &anticlockwise", [this]() { editor->rotateCCW(); },
                  QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::Key_R));
  edit->addAction("&Flip left–right", [this]() { editor->flipH(); },
                  QKeySequence(Qt::CTRL + Qt::Key_F));
  edit->addAction("Flip &up–down", [this]() { editor->flipV(); },
                  QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::Key_F));
  edit->addAction("&Group", [this]() { editor->formGroup(); },
		  QKeySequence(Qt::CTRL + Qt::Key_G));
  edit->addAction("&Ungroup", [this]() { editor->dissolveGroup(); },
		  QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::Key_G));
  edit->addAction("&Delete selected", [this]() { editor->deleteSelected(); },
		  QKeySequence(Qt::Key_Delete));
  edit->addAction("&Undo", [this]() { editor->undo(); },
		  QKeySequence(Qt::CTRL + Qt::Key_Z));
  edit->addAction("&Redo", [this]() { editor->redo(); },
		  QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::Key_Z));

  auto *view = mb->addMenu("&View");
  view->addAction("&Scale to fit", [this]() { editor->scaleToFit(); },
		  QKeySequence(Qt::CTRL + Qt::Key_0));
  view->addAction("Zoom &in", [this]() { editor->zoomIn(); },
		  QKeySequence(Qt::CTRL + Qt::Key_Equal));
  view->addAction("Zoom &out", [this]() { editor->zoomOut(); },
		  QKeySequence(Qt::CTRL + Qt::Key_Minus));
  
  auto *help = mb->addMenu("&Help");
  help->addAction("&About", [this]() { about(); });
}

void MWData::makeEditor() {
  editor = new Editor(mw);
  mw->setCentralWidget(editor);
}

void MWData::makeConnections() {
  // Editor to status bar and v.v.
  QObject::connect(editor, &Editor::hovering,
		   statusbar, &Statusbar::setCursorXY);
  QObject::connect(editor, &Editor::leaving,
		   statusbar, &Statusbar::hideCursorXY);
  QObject::connect(editor, &Editor::boardChanged,
		   statusbar, &Statusbar::setBoard);
  QObject::connect(statusbar, &Statusbar::gridEdited,
		   editor, &Editor::setGrid);
  QObject::connect(statusbar, &Statusbar::layerVisibilityEdited,
		   editor, &Editor::setLayerVisibility);
  QObject::connect(statusbar, &Statusbar::planesVisibilityEdited,
		   editor, &Editor::setPlanesVisibility);

  // Editor to properties bar
  QObject::connect(editor, &Editor::selectionChanged,
		   propbar, &Propertiesbar::reflectSelection);
  QObject::connect(editor, &Editor::boardChanged,
		   propbar, &Propertiesbar::reflectBoard);

  // Mode bar to others
  QObject::connect(modebar, &Modebar::modeChanged,
		   propbar, &Propertiesbar::reflectMode);
  QObject::connect(modebar, &Modebar::modeChanged,
		   editor, &Editor::setMode);
}

void MWData::fillBars() {
  statusbar->setBoard(editor->pcbLayout().board());
  propbar->reflectBoard(editor->pcbLayout().board());
  propbar->reflectMode(modebar->mode());
  propbar->reflectSelection();
  propbar->forwardAllProperties();
}

MainWindow::MainWindow(): QMainWindow() {
  d = new MWData(this);
  d->makeEditor();
  d->makeToolbars();
  d->makeMenus();
  d->makeConnections();
  d->fillBars();
}

MainWindow::~MainWindow() {
  delete d;
}
