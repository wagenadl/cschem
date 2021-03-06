// MainWindow.cpp

#include "MainWindow.h"
#include "Modebar.h"
#include "Propertiesbar.h"
#include "Statusbar.h"
#include "Editor.h"
#include "MultiCompView.h"
#include <QCloseEvent>
#include "data/Paths.h"
#include "gerber/GerberWriter.h"
#include "gerber/PasteMaskWriter.h"
#include "Find.h"
#include "Settings.h"
#include "BoardSizeDialog.h"
#include "data/NetMismatch.h"
#include "Version.h"
#include "gerber/FrontPanelWriter.h"
#include "ORenderer.h"
#include "BOM.h"
#include "BOMView.h"

#include <QClipboard>
#include <QTemporaryDir>
#include <QDesktopServices>
#include <QInputDialog>
#include <QProcess>
#include <QMessageBox>
#include <QMenu>
#include <QMenuBar>
#include <QApplication>
#include <QFileDialog>
#include <QDockWidget>
#include <QBitmap>
#include <QPainter>

 

class MWData {
public:
  MWData(MainWindow *mw): mw(mw) {
    editor = 0;
    mcv = 0;
    mcvdock = 0;
    bomv = 0;
    bomvdock = 0;
  }
  void setWindowTitle();
  void resetFilename();
  void about();
  void makeToolbars();
  void makeMenus();
  void makeParts();
  void makeBOM();
  void makeEditor();
  void makeConnections();
  void showParts();
  void showBOM();
  void fillBars();
  void boardSizeDialog();
  void openDialog();
  void newWindow();
  void arbitraryRotation();
  bool exportAsDialog();
  bool exportPasteMaskDialog();
  bool exportFrontPanelDialog();
  bool exportShoppingListDialog();
  bool exportBOMDialog();
  bool importBOMDialog();
  bool saveAsDialog();
  bool saveImmediately();
  void copyPCBImage(bool forprinting=true);
  void linkSchematicDialog();
  void insertComponentDialog();
  void openLibrary();
  void saveComponentDialog();
  void verifyNets();
  void selectionToBOM();
  void selectionFromBOM();
  QString getSaveFilename(QString ext, QString caption); // updates pwd
  QString getOpenFilename(QString ext, QString caption, QString desc="");
  // updates pwd
public:
  MainWindow *mw;
  Modebar *modebar;
  Propertiesbar *propbar;
  Statusbar *statusbar;
  MultiCompView *mcv;
  QDockWidget *mcvdock;
  BOMView *bomv;
  QDockWidget *bomvdock;
  Editor *editor;
  QString pwd;
  QString compwd;
  QString filename;
};

QString MWData::getOpenFilename(QString ext, QString caption, QString desc) {
  if (pwd.isEmpty())
    pwd = Paths::defaultLocation();

  if (desc=="")
    desc = ext.toUpper() + " files (*." + ext + ")";

  QString fn = QFileDialog::getOpenFileName(0, caption, pwd, desc);
  if (fn.isEmpty())
    return "";

  pwd = QFileInfo(fn).absolutePath();
  return QFileInfo(fn).absoluteFilePath();
}


QString MWData::getSaveFilename(QString ext, QString caption) {
  if (pwd.isEmpty())
    pwd = Paths::defaultLocation();

  QString path = pwd;
  if (!filename.isEmpty()) {
    if (!path.endsWith("/"))
      path += "/";
    path += QFileInfo(filename).baseName();
    path += "." + ext;
  }
  
  QString fn = QFileDialog::getSaveFileName(0, caption,
					    path,
					    ext.toUpper() + " files (*."
                                            + ext + ")");
  if (fn.isEmpty())
    return "";

  if (!fn.endsWith("." + ext))
    fn += "." + ext;
  pwd = QFileInfo(fn).absolutePath();
  return QFileInfo(fn).absoluteFilePath();
}

void MWData::resetFilename() {
  filename = "";
  setWindowTitle();
}

void MWData::setWindowTitle() {
  QString lbl = filename;
  if (editor && !editor->isAsSaved()) {
    if (lbl.isEmpty())
      lbl = "(Untitled) *";
    else
      lbl += " *";
  } else {
    if (lbl.isEmpty())
      lbl = "CPCB";
  }
  mw->setWindowTitle(lbl);
}

void MWData::boardSizeDialog() {
  BoardSizeDialog bsd;
  bsd.setLayout(editor->pcbLayout());
  if (bsd.exec()) {
    editor->setBoardSize(bsd.boardWidth(), bsd.boardHeight(), bsd.boardShape());
  }
}

void MWData::showParts() {
  if (!mcv)
    makeParts();
  if (mcvdock->isVisible()) { 
      mcvdock->hide();
  } else {
    mcvdock->show();
    mw->addDockWidget(Qt::LeftDockWidgetArea, mcvdock);
    mcv->setSchem(editor->linkedSchematic().schematic());
    mcv->setRoot(editor->pcbLayout().root());
  }
}

void MWData::showBOM() {
  if (!bomv)
    makeBOM();
  if (bomvdock->isVisible()) {
    bomvdock->hide();
  } else {
    bomvdock->show();
    mw->addDockWidget(Qt::RightDockWidgetArea, bomvdock);
  }
}

void MWData::newWindow() {
  auto *w = new MainWindow();
  w->resize(mw->size());
  w->show();
}

void MWData::openDialog() {
  QString fn = getOpenFilename("cpcb", "Select file to open…",
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
     compwd = Paths::userComponentRoot();
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

void MWData::openLibrary() {
  QDesktopServices::openUrl(QUrl(Paths::userComponentRoot()));
}

void MWData::insertComponentDialog() {
  if (compwd.isEmpty()) {
    compwd = Paths::userComponentRoot();
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

void MWData::verifyNets() {
  Group const &grp = editor->pcbLayout().root();
  QSet<QString> dupgroups = grp.duplicatedGroupRefs();
  if (!dupgroups.isEmpty()) {
    QString msg = "The following part refs. are duplicated: ";
    for (QString n: dupgroups) 
      msg += "\n  " + n;
    QMessageBox::warning(0, "cpcb", msg);
    return;
  }
  QMap<QString, QSet<QString>> duppins = grp.groupsWithDuplicatedPins();
  if (!duppins.isEmpty()) {
    auto it = duppins.begin();
    QString msg = "Part " + it.key() + " has duplicated pin names:";
    for (QString n: it.value())
      msg += "\n  " + n;
    QMessageBox::warning(0, "cpcb", msg);
    return;
  }
    
  NetMismatch nm;
  nm.recalculateAll(editor->linkedSchematic(), grp);
  nm.report(grp);
  if (!nm.wronglyInNet.isEmpty()) {
    editor->pretendOnNet(*nm.wronglyInNet.begin());
  } else if (!nm.missingFromNet.isEmpty()) {
    editor->pretendOnNet(*nm.missingFromNet.begin());
  }
  QStringList names;
  for (Nodename nn: nm.missingEntirely)
    names << nn.humanName();
  statusbar->setMissing(names);
  if (nm.wronglyInNet.isEmpty() && nm.missingFromNet.isEmpty()
      && nm.missingEntirely.isEmpty())
    QMessageBox::information(0, "cpcb", "All nets verified OK.");
}

void MWData::linkSchematicDialog() {
  QString fn = getOpenFilename("cschem", "Link schematic…",
                               "Schematics (*.schem *.cschem)");
  if (fn.isEmpty())
    return;

  if (editor->linkSchematic(fn))
    showParts();
  else
    QMessageBox::warning(mw, "Failed to link schematic",
                         "Cannot link schematic “" + fn
                         + "”. Could the file be damaged?");
}

void MWData::arbitraryRotation() {
  int angle = QInputDialog::getInt(mw, "Arbitrary rotation",
				   "Clockwise angle (degrees):",
				   0, -359, 359);
  if (angle)
    editor->arbitraryRotation(angle);
}

void MWData::copyPCBImage(bool forprinting) {
  QPixmap img(editor->size());
  editor->render(&img);
  if (forprinting) {
    QPainter p(&img);
    QBitmap msk = img.createMaskFromColor(ORenderer::backgroundColor(),
					  Qt::MaskOutColor);
    p.setPen(QColor(255, 255, 255));
    p.drawPixmap(img.rect(), msk, msk.rect());
    msk = img.createMaskFromColor(ORenderer::boardColor(), Qt::MaskOutColor);
    p.setPen(QColor(255, 255, 255));
    p.drawPixmap(img.rect(), msk, msk.rect());
    msk = img.createMaskFromColor(layerColor(Layer::Silk), Qt::MaskOutColor);
    p.setPen(QColor(0, 0, 0));
    p.drawPixmap(img.rect(), msk, msk.rect());
    p.end();
  }
  QApplication::clipboard()->setPixmap(img);
}

bool MWData::exportPasteMaskDialog() {
  QString fn = getSaveFilename("svg", "Export paste mask as…");
  if (fn.isEmpty())
    return false;

  bool metric = editor->pcbLayout().board().isEffectivelyMetric();
  QString unit = metric ? "mm" : "inch";
  int decimals = metric ? 2 : 3;
  double max = metric ? 1 : .05;
  Settings stg;
  Dim dflt = Dim::fromString(stg.value("shrinkage",
                                       Dim::fromInch(0.005).toString())
                             .toString());
  double shrinkage = QInputDialog::getDouble(mw, "Export paste mark",
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

bool MWData::exportShoppingListDialog() {
  QString fn = getSaveFilename("csv", "Export shopping list as…");
  if (fn.isEmpty())
    return false;
  return editor->bom()->saveShoppingListAsCSV(fn);
}

bool MWData::exportBOMDialog() {
  QString fn = getSaveFilename("csv", "Export BOM as…");
  if (fn.isEmpty())
    return false;
  return editor->bom()->saveAsCSV(fn);
}

bool MWData::importBOMDialog() {
  QString fn = getOpenFilename("csv", "Import BOM…");
  if (fn.isEmpty())
    return false;
  return editor->loadBOM(fn);
}
  

bool MWData::exportFrontPanelDialog() {
  QString fn = getSaveFilename("svg", "Export front panel as…");
  if (fn.isEmpty())
    return false;

  FrontPanelWriter pmw;
  if (pmw.write(editor->pcbLayout(), fn))
    return true;

  QMessageBox::warning(mw, "cpcb",
		       "Could not export front panel as “"
		       + fn + "”",
		       QMessageBox::Ok);
  return false;
}


bool MWData::exportAsDialog() {
  QString fn = getSaveFilename("zip", "Export as Gerber…");
  if (fn.isEmpty())
    return false;
  
  QString base = QFileInfo(fn).baseName();
  QTemporaryDir td;
  bool ok = false;
  if (td.isValid()) {
    if (GerberWriter::write(editor->pcbLayout(), td.filePath(base))) {
      QDir cwd = QDir::current();
      QDir::setCurrent(td.path());
      QStringList args;
      args << "-r" << fn << base;
      QDir::root().remove(fn);
      ok = QProcess::execute("zip", args)==0;
      QDir::setCurrent(cwd.absolutePath());
    }
  }
  QDir(td.filePath(base)).removeRecursively();
  if (!ok)
    QMessageBox::warning(mw, "cpcb",
                         "Could not export pcb as “"
                         + fn + "”",
                         QMessageBox::Ok);
  return ok;
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
  pwd = QFileInfo(fn).absolutePath();
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
  QString vsn = Version::toString();
  QMessageBox::about(0, "About " + me,
		     me + " " + vsn
		     + "<p>" + "(C) 2018–2019 Daniel A. Wagenaar\n"
		     + "<p>" + me + " is a program for printed circuit board  layout. More information is available at <a href=\"http://www.danielwagenaar.net/cschem\">www.danielwagenaar.net/cschem</a>.\n"
		     + "<p>" + me + " is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.\n"
		     + "<p>" + me + " is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.\n"
		     + "<p>" + "You should have received a copy of the GNU General Public License along with this program. If not, see <a href=\"http://www.gnu.org/licenses/gpl-3.0.en.html\">www.gnu.org/licenses/gpl-3.0.en.html</a>.");
}

void MWData::makeParts() {
  Q_ASSERT(editor);
  mcvdock = new QDockWidget("Parts to be placed", mw);
  mcv = new MultiCompView;
  mcv->setScale(editor->pixelsPerMil());
  mcv->linkEditor(editor);
  QObject::connect(editor, &Editor::componentsChanged,
		   [this]() { mcv->setRoot(editor->pcbLayout().root()); });
  QObject::connect(editor, &Editor::scaleChanged,
		   [this]() { mcv->setScale(editor->pixelsPerMil()); });
  mcvdock->setWidget(mcv);
  showParts();
}

void MWData::makeBOM() {
  Q_ASSERT(editor);
  bomvdock = new QDockWidget("BOM", mw);
  bomv = new BOMView;
  bomvdock->setWidget(bomv);
  bomv->setModel(editor->bom());
  bomvdock->hide();
  //  showBOM();
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
  file->addAction("&New", [this]() { newWindow(); },
		  QKeySequence(Qt::CTRL + Qt::Key_N));

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

  file->addAction("Export &front panel…", [this]() { exportFrontPanelDialog(); },
		  QKeySequence(Qt::ALT + Qt::CTRL + Qt::Key_E));

  file->addAction("Copy PCB &image to clipboard", [this]() { copyPCBImage(); },
		  QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::Key_C));
		  
  file->addAction("Export &BOM as CSV…",  [this]() { exportBOMDialog(); });
  file->addAction("Export shopping &list as CSV…",  [this]() {
                           exportShoppingListDialog();
                                                    });
  file->addAction("&Import BOM from CSV…",  [this]() { importBOMDialog(); });
  
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

  a = edit->addAction("Select attached &trace",
		      [this]() { editor->selectTrace(false); },
		      QKeySequence(Qt::CTRL + Qt::Key_T));
  QObject::connect(editor, &Editor::selectionChanged,
		   a, &QAction::setEnabled);
  a->setEnabled(false);

  a = edit->addAction("Select attached &net",
		      [this]() { editor->selectTrace(true); },
		      QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::Key_T));
  QObject::connect(editor, &Editor::selectionChanged,
		   a, &QAction::setEnabled);
  a->setEnabled(false);

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

  a = edit->addAction("Arbitrary rotatio&n…",
		      [this]() { arbitraryRotation(); },
		      QKeySequence(Qt::ALT + Qt::Key_R));
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
  //QObject::connect(editor, &Editor::selectionChanged,
  //a, &QAction::setEnabled);
  //a->setEnabled(false);
  
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

  auto *tools = mb->addMenu("&Tools");
  tools->addAction("&Open library", [this]() { openLibrary(); },
		  QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::Key_O));
  tools->addAction("&Insert component…", [this]() { insertComponentDialog(); },
		   QKeySequence(Qt::CTRL + Qt::Key_I));
  a = tools->addAction("Save &component…", [this]() { saveComponentDialog(); },
		      QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::Key_I));
  QObject::connect(editor, &Editor::selectionIsGroup,
		   a, &QAction::setEnabled);
  a->setEnabled(false);
  tools->addAction("&Board size…",
		   [this]() { boardSizeDialog(); });
		   
  tools->addAction("Remove &dangling traces",
		   [this]() { editor->deleteDanglingTraces(); },
		   QKeySequence(Qt::CTRL + Qt::Key_B));

  tools->addAction("Cleanup &trace intersections",
		   [this]() { editor->cleanupIntersections(); });
  
  tools->addAction("&Link schematic…", [this]() { linkSchematicDialog(); },
		  QKeySequence(Qt::CTRL + Qt::Key_L));

  a = tools->addAction("&Unlink schematic",
		      [this]() { editor->unlinkSchematic(); },
		      QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::Key_L));
  QObject::connect(editor, &Editor::schematicLinked,
		   a, &QAction::setEnabled);
  a->setEnabled(false);
  a = tools->addAction("&Verify nets",
		      [this]() { verifyNets(); },
		      QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::Key_V));
  QObject::connect(editor, &Editor::schematicLinked,
		   a, &QAction::setEnabled);
  a->setEnabled(false);
  
  auto *view = mb->addMenu("&View");
  view->addAction("&Scale to fit", [this]() { editor->scaleToFit(); },
		  QKeySequence(Qt::Key_0));
  view->addAction("Zoom &in", [this]() { editor->zoomIn(); },
		  QKeySequence(Qt::Key_Equal));
  view->addAction("Zoom &out", [this]() { editor->zoomOut(); },
		  QKeySequence(Qt::Key_Minus));
  view->addAction("Show &parts to be placed", [this]() { showParts(); },
		      QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::Key_P));
  view->addAction("Show &BOM", [this]() { showBOM(); },
		      QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::Key_B));
  
  auto *help = mb->addMenu("&Help");
  help->addAction("&About", [this]() { about(); });
}

void MWData::makeEditor() {
  editor = new Editor(mw);
  mw->setCentralWidget(editor);
}

void MWData::makeConnections() {
  // Editor to status bar and v.v.
  QObject::connect(editor, &Editor::userOriginChanged,
		   [this](Point o) {
		     if (!modebar->isOriginIncremental())
		       o = Point();
		     statusbar->setUserOrigin(o);
		     propbar->setUserOrigin(o);
		     modebar->setMode(Mode::Edit);
		   });
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
  QObject::connect(editor, &Editor::tentativeMove,
		   propbar, &Propertiesbar::reflectTentativeMove);

  // Mode bar to others
  QObject::connect(modebar, &Modebar::modeChanged,
		   editor, &Editor::setMode);
  QObject::connect(modebar, &Modebar::modeChanged,
		   propbar, &Propertiesbar::reflectMode);
  QObject::connect(modebar, &Modebar::constraintChanged,
		   editor, &Editor::setAngleConstraint);
  QObject::connect(modebar, &Modebar::originChanged,
		   [this](bool inc) {
		     Point o = inc ? editor->userOrigin() : Point();
		     statusbar->setUserOrigin(o);
		     propbar->setUserOrigin(o);
		   });
  QObject::connect(modebar, &Modebar::modeChanged,
		   [this](Mode m) {
                     if (m==Mode::PlacePlane) {
                       qDebug() << "I might make the planes visible now";
                       // statusbar->showPlanes();
                     }
                   });
  

  // Editor to us
  QObject::connect(editor, &Editor::changedFromSaved,
		   [this]() { setWindowTitle(); });

  // Selection between editor and BOM
  QObject::connect(editor, &Editor::selectionChanged,
                   [this]() { selectionToBOM(); });
  QObject::connect(bomv->selectionModel(),
                   &QItemSelectionModel::selectionChanged,
                   [this]() { selectionFromBOM(); });
}

void MWData::fillBars() {
  statusbar->setBoard(editor->pcbLayout().board());
  propbar->reflectBoard(editor->pcbLayout().board());
  propbar->reflectMode(modebar->mode());
  qDebug() << "mw:fillbars";
  propbar->reflectSelection();
  propbar->forwardAllProperties();
}

MainWindow::MainWindow(): QMainWindow() {
  d = new MWData(this);
  d->makeEditor();
  d->makeToolbars();
  d->makeBOM();
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

void MWData::selectionToBOM() {
  qDebug() << "selection to bom";
  if (editor->breadcrumbs().size())
    return; // we only care at top level
  QSet<int> sel = editor->selectedObjects();
  bomv->selectElements(sel);
}

void MWData::selectionFromBOM() {
  qDebug() <<"selection from bom";
  if (editor->breadcrumbs().size())
    return; // we only care at top level
  QSet<int> sel = bomv->selectedElements();
  editor->select(sel);
}
