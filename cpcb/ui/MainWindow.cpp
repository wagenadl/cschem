// MainWindow.cpp

#include "MainWindow.h"
#include "Modebar.h"
#include "Propertiesbar.h"
#include "Statusbar.h"
#include "Editor.h"
#include "MultiCompView.h"
#include "data/Paths.h"
#include "gerber/GerberWriter.h"
#include "gerber/PasteMaskWriter.h"
#include "Find.h"
#include "Settings.h"

#include <QInputDialog>
#include <QProcess>
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
  void setWindowTitle();
  void resetFilename();
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
  bool exportAsDialog();
  bool exportPasteMaskDialog();
  bool saveAsDialog();
  bool saveImmediately();
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

void MWData::resetFilename() {
  filename = "Untitled.cpcb";
  setWindowTitle();
}

void MWData::setWindowTitle() {
  QString lbl = filename;
  if (editor && !editor->isAsSaved())
    lbl += " *";
  mw->setWindowTitle(lbl);
}

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
  mcv->setSchem(editor->linkedSchematic().schematic());
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
    w->open(fn);
    w->show();
  }
}

void MainWindow::open(QString fn) {
  if (d->editor->load(fn)) {
    d->filename = fn;
    d->pwd = QFileInfo(fn).dir().absolutePath();
    setWindowTitle(fn);
    if (d->editor->linkedSchematic().isValid())
      d->showParts();
  } else {
    d->resetFilename();
  }
}  

bool MWData::saveImmediately() {
  if (filename.isEmpty()) {
    return saveAsDialog();
  } else {
    bool ok = editor->save(filename);
    if (!ok)
      QMessageBox::warning(mw, "cpcb",
			   "Could not save pcb as “"
			   + filename + "”",
			   QMessageBox::Ok);
    return ok;
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

bool MWData::exportPasteMaskDialog() {
  if (pwd.isEmpty())
    pwd = Paths::defaultLocation();

  QString path = pwd;
  if (!filename.isEmpty()) {
    if (!path.endsWith("/"))
      path += "/";
    path += QFileInfo(filename).baseName();
    path += ".svg";
  }
  
  QString fn = QFileDialog::getSaveFileName(0, "Export paste mask as…",
					    path,
					    "SVG files (*.svg)");
  if (fn.isEmpty())
    return false;
  if (!fn.endsWith(".svg"))
    fn += ".svg";

  bool metric = editor->pcbLayout().board().isEffectivelyMetric();
  QString unit = metric ? "mm" : "inch";
  int decimals = metric ? 2 : 3;
  double max = metric ? 1 : .05;
  Settings stg;
  Dim dflt = Dim::fromString(stg.value("shrinkage",
                                       Dim::fromInch(0.005).toString())
                             .toString());
  double shrinkage = QInputDialog::getDouble(mw, "Export Paste mark",
					     "Shrinkage for cutouts ("
					     + unit + "):",
					     metric ? dflt.toMM()
                                             : dflt.toInch(),
                                             0, max, decimals);
  Dim shrnk = metric ? Dim::fromMM(shrinkage)
    : Dim::fromInch(shrinkage);
  if (shrnk != dflt)
    stg.setValue("shrinkage", shrnk.toString());
  
  PasteMaskWriter pmw;
  pmw.setShrinkage(shrnk);
  if (pmw.write(editor->pcbLayout(), fn))
    return true;

  QMessageBox::warning(mw, "cpcb",
		       "Could not export paste mask as “"
		       + fn + "”",
		       QMessageBox::Ok);
  return false;
}
  

bool MWData::exportAsDialog() {
  if (pwd.isEmpty())
    pwd = Paths::defaultLocation();

  QString path = pwd;
  if (!filename.isEmpty()) {
    if (!path.endsWith("/"))
      path += "/";
    path += QFileInfo(filename).baseName();
    path += ".zip";
  }
  QString fn = QFileDialog::getSaveFileName(0, "Export as…",
					    path,
					    "Zip files (*.zip)");
  if (fn.isEmpty())
    return false;
  if (fn.endsWith(".zip"))
    fn=fn.left(fn.size()-4);
  if (GerberWriter::write(editor->pcbLayout(), fn)) {
    QString zipfn = fn + ".zip";
    QStringList args;
    args << "-r" << zipfn <<fn;
    QDir::root().remove(zipfn);
    if (QProcess::execute("zip", args)==0) {
      // rm the folder
      return true;
    }
  }

  QMessageBox::warning(mw, "cpcb",
		       "Could not export pcb as “"
		       + fn + "”",
		       QMessageBox::Ok);
  return false;
}

bool MWData::saveAsDialog() {
  if (pwd.isEmpty())
    pwd = Paths::defaultLocation();
  
  QString fn = QFileDialog::getSaveFileName(0, "Save as…",
					    pwd,
					    "PCB layouts (*.cpcb)");
  if (fn.isEmpty())
    return false;

  if (!fn.endsWith(".cpcb"))
    fn += ".cpcb";
  filename = fn;
  pwd = QFileInfo(fn).dir().absolutePath();
  mw->setWindowTitle(fn);

  if (editor->save(fn))
    return true;

  QMessageBox::warning(mw, "cpcb",
		       "Could not save pcb as “"
		       + fn + "”",
		       QMessageBox::Ok);
  return false;
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
  mcv->setScale(editor->pixelsPerMil());
  Q_ASSERT(editor);
  QObject::connect(editor, &Editor::componentsChanged,
		   [this]() { mcv->setRoot(editor->pcbLayout().root()); });
  QObject::connect(editor, &Editor::scaleChanged,
		   [this]() { mcv->setScale(editor->pixelsPerMil()); });
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

  QAction *a;
  
  auto *file = mb->addMenu("&File");
  file->addAction("&Open…", [this]() { openDialog(); },
		  QKeySequence(Qt::CTRL + Qt::Key_O));

  a = file->addAction("&Save", [this]() { saveImmediately(); },
		      QKeySequence(Qt::CTRL + Qt::Key_S));
  QObject::connect(editor, &Editor::changedFromSaved,
		   a, &QAction::setEnabled);

  file->addAction("Save &as…", [this]() { saveAsDialog(); },
		  QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::Key_S));

  file->addAction("&Export Gerber…", [this]() { exportAsDialog(); },
		  QKeySequence(Qt::CTRL + Qt::Key_E));

  file->addAction("Export &paste mask…", [this]() { exportPasteMaskDialog(); },
		  QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::Key_E));

  
  file->addAction("&Link schematic…", [this]() { linkSchematicDialog(); },
		  QKeySequence(Qt::CTRL + Qt::Key_L));

  a = file->addAction("&Unlink schematic",
		      [this]() { editor->unlinkSchematic(); },
		      QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::Key_L));
  QObject::connect(editor, &Editor::schematicLinked,
		   a, &QAction::setEnabled);
  a->setEnabled(false);
  
  file->addAction("&Insert component…", [this]() { insertComponentDialog(); },
		  QKeySequence(Qt::CTRL + Qt::Key_I));

  a = file->addAction("Save &component…", [this]() { saveComponentDialog(); },
		      QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::Key_I));
  QObject::connect(editor, &Editor::selectionIsGroup,
		   a, &QAction::setEnabled);
  a->setEnabled(false);
  
  file->addAction("&Quit", []() { QApplication::quit(); });

  auto *edit = mb->addMenu("&Edit");

  a = edit->addAction("&Cut", [this]() { editor->cut(); },
		      QKeySequence(Qt::CTRL + Qt::Key_X));
  QObject::connect(editor, &Editor::selectionChanged,
		   a, &QAction::setEnabled);
  a->setEnabled(false);
  
  a = edit->addAction("Cop&y", [this]() { editor->copy(); },
		      QKeySequence(Qt::CTRL + Qt::Key_C));
  QObject::connect(editor, &Editor::selectionChanged,
		   a, &QAction::setEnabled);
  a->setEnabled(false);
  
  edit->addAction("&Paste", [this]() { editor->paste(); },
		  QKeySequence(Qt::CTRL + Qt::Key_V));

  a = edit->addAction("&Rotate clockwise", [this]() { editor->rotateCW(); },
		      QKeySequence(Qt::CTRL + Qt::Key_R));
  QObject::connect(editor, &Editor::selectionChanged,
		   a, &QAction::setEnabled);
  a->setEnabled(false);
  
  a = edit->addAction("Rotate &anticlockwise",
		      [this]() { editor->rotateCCW(); },
		      QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::Key_R));
  QObject::connect(editor, &Editor::selectionChanged,
		   a, &QAction::setEnabled);
  a->setEnabled(false);
  
  a = edit->addAction("&Flip left–right", [this]() { editor->flipH(); },
		      QKeySequence(Qt::CTRL + Qt::Key_F));
  QObject::connect(editor, &Editor::selectionChanged,
		   a, &QAction::setEnabled);
  a->setEnabled(false);
  
  a = edit->addAction("Flip up–down", [this]() { editor->flipV(); },
                  QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::Key_F));
  QObject::connect(editor, &Editor::selectionChanged,
		   a, &QAction::setEnabled);
  a->setEnabled(false);
  
  a = edit->addAction("&Group", [this]() { editor->formGroup(); },
		      QKeySequence(Qt::CTRL + Qt::Key_G));
  QObject::connect(editor, &Editor::selectionChanged,
		   a, &QAction::setEnabled);
  a->setEnabled(false);
  
  a = edit->addAction("&Ungroup", [this]() { editor->dissolveGroup(); },
		      QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::Key_G));
  QObject::connect(editor, &Editor::selectionIsGroup,
		   a, &QAction::setEnabled);
  a->setEnabled(false);
  
  a = edit->addAction("&Delete selected",
		      [this]() { editor->deleteSelected(); },
		      QKeySequence(Qt::Key_Delete));
  QObject::connect(editor, &Editor::selectionChanged,
		   a, &QAction::setEnabled);
  a->setEnabled(false);
  
  a = edit->addAction("&Undo", [this]() { editor->undo(); },
		      QKeySequence(Qt::CTRL + Qt::Key_Z));
  QObject::connect(editor, &Editor::undoAvailable,
		   a, &QAction::setEnabled);
  a->setEnabled(false);
  
  a = edit->addAction("Redo", [this]() { editor->redo(); },
		      QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::Key_Z));
  QObject::connect(editor, &Editor::redoAvailable,
		   a, &QAction::setEnabled);
  a->setEnabled(false);

  edit->addAction("Find", [this]() { Find(editor).run(); },
                  QKeySequence(Qt::CTRL + Qt::Key_Slash));
  
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
  QObject::connect(editor, &Editor::onObject,
		   statusbar, &Statusbar::setObject);
  QObject::connect(editor, &Editor::missingNodes,
		   statusbar, &Statusbar::setMissing);
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
  QObject::connect(statusbar, &Statusbar::netsVisibilityEdited,
		   editor, &Editor::setNetsVisibility);

  // Editor to properties bar
  QObject::connect(editor, &Editor::selectionChanged,
		   propbar, &Propertiesbar::reflectSelection);
  QObject::connect(editor, &Editor::boardChanged,
		   propbar, &Propertiesbar::reflectBoard);
  QObject::connect(editor, &Editor::insertedPadOrHole,
                   propbar, &Propertiesbar::stepPinNumber);

  // Mode bar to others
  QObject::connect(modebar, &Modebar::modeChanged,
		   propbar, &Propertiesbar::reflectMode);
  QObject::connect(modebar, &Modebar::modeChanged,
		   editor, &Editor::setMode);
  QObject::connect(modebar, &Modebar::constraintChanged,
		   editor, &Editor::setAngleConstraint);
  

  // Editor to us
  QObject::connect(editor, &Editor::changedFromSaved,
		   [this]() { setWindowTitle(); });
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
  d->resetFilename();
}

MainWindow::~MainWindow() {
  delete d;
}

void MainWindow::closeEvent(QCloseEvent *e) {
  if (d->editor->isAsSaved()) {
    e->accept();
  } else {
    auto ret
      = QMessageBox::warning(this, "cpcb",
			     tr("The layout has been modified.\n"
				"Do you want to save your changes?"),
			     QMessageBox::Save
			     | QMessageBox::Discard
			     | QMessageBox::Cancel);
    switch (ret) {
    case QMessageBox::Save:
      if (d->saveImmediately())
	e->accept();
      else
	e->ignore();
      break;
    case QMessageBox::Cancel:
      e->ignore();
      break;
    case QMessageBox::Discard:
      e->accept();
      break;
    default:
      e->accept();
      break;
    }
  }
}
