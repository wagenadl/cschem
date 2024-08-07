// MainWindow.cpp

#include "MainWindow.h"
#include "ExportDialog.h"
#include "Modebar.h"
#include "Propertiesbar.h"
#include "Statusbar.h"
#include "Editor.h"
#include "MultiCompView.h"
#include "CircularPatternDialog.h"
#include "LinearPatternDialog.h"
#include <QCloseEvent>
#include "data/Paths.h"
#include "Find.h"
#include <QDesktopServices>
#include <QSettings>
#include "BoardSizeDialog.h"
#include <QLabel>
#include <QWidgetAction>
#include "data/NetMismatch.h"
#include "Version.h"
#include "ORenderer.h"
#include "BOM.h"
#include "BOMView.h"
#include <QPushButton>
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
#include "ui/RecentFiles.h"
#include <QMetaMethod> 

static QMap<QString, MainWindow *> &openFiles() {
  static QMap<QString, MainWindow *> *m = 0;
  if (m==0)
    m = new QMap<QString, MainWindow *>();
  return *m;
}

class MWData {
public:
  MWData(MainWindow *mw): mw(mw) {
    editor = 0;
    mcv = 0;
    mcvdock = 0;
    bomv = 0;
    bomvdock = 0;
    recentfiles = 0;
    linklabel = 0;
  }
  void discussRelinkWhileSaving();
  void attemptRelinkSchematic(); 
  void discussRelinkWhileLoading(); 
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
  void revert();
  void newWindow();
  void arbitraryRotation();
  bool exportDialog();
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
  void openLinkedSchematic();
  QString getSaveFilename(QString ext, QString caption); // updates pwd
  QString getOpenFilename(QString ext, QString caption, QString desc="");
  // updates pwd
  void passthroughSignal(QString sig);
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
  RecentFiles *recentfiles;
  QLabel *linklabel;
};

void MWData::passthroughSignal(QString sig) {
  /* This technique based on
     https://srivatsp.com/ostinato/qt-cut-copy-paste/
  */
  QWidget *dest = qApp->focusWidget();
  if (!dest)
    return;
  QMetaObject const *meta = dest->metaObject();
  int idx = meta->indexOfSlot(qPrintable(sig + "()"));
  if (idx>=0)
    meta->method(idx).invoke(dest, Qt::AutoConnection);
}

void MWData::discussRelinkWhileSaving() {
  QString schemfn = editor->pcbLayout().board().linkedschematic;
  if (schemfn=="")
    return; // not linked to anythign
  QFileInfo schemfi(schemfn);
  QString oldleaf = schemfi.fileName();
  QFileInfo myfi(filename);
  QString newleaf = myfi.fileName();
  if (newleaf.endsWith(".cpcb"))
    newleaf = newleaf.left(newleaf.size() - 5);
  newleaf += ".cschem";
  QDir dir = schemfi.dir();
  if (!dir.exists(newleaf))
    return; // no new schematic exists

  QString txt = "Your layout is linked to a schematic named “" + oldleaf + "”.";
  txt += " However, a schematic named “" + newleaf + "” also exists";
  txt += " in “" + dir.absolutePath() + "”.";
  txt += " Would you like to link to that schematic instead?";

  QMessageBox mb(QMessageBox::Question, "CPCB", txt,
                 QMessageBox::No | QMessageBox::Yes, mw);
  
  if (mb.exec() != QMessageBox::Yes)
    return;
  
  QString altfn = dir.absoluteFilePath(newleaf);
  if (!editor->linkSchematic(altfn))
      QMessageBox::warning(mw, "Failed to link schematic",
                           "Cannot link schematic “" + altfn
                           + "”. Could the file be damaged?");
}

void MWData::attemptRelinkSchematic() {
  /* If the linked schematic could not be found at original location,
     check whether a file with the same name exists locally, and offer
     to load it. Alternatively, suggest that the user browse for it.
   */
  QString schemfn = editor->pcbLayout().board().linkedschematic;
  QString txt = "Could not load linked schematic “" + schemfn + "”.";
  QString leaf = QFileInfo(schemfn).fileName();
  QDir here = QFileInfo(filename).dir();
  bool available = here.exists(leaf);
  if (available) 
    txt += " However, a file named “" + leaf + "” exists in “"
      + here.canonicalPath() + "”."
      + " Is that the correct file to use?"
      + " Alternatively, would you like to browse for the correct schematic?";
  else 
    txt += " Would you like to browse for it?";

  QMessageBox mb(QMessageBox::Question, "CPCB", txt, QMessageBox::NoButton, mw);
  
  QPushButton *yes = available ? mb.addButton("Yes", QMessageBox::YesRole) : 0;
  QPushButton *browse = mb.addButton("Browse", QMessageBox::YesRole);
  /*QPushButton *no =*/ mb.addButton("No", QMessageBox::NoRole);

  mb.exec();
  
  if (yes && mb.clickedButton()==yes) {
    QString fn = here.filePath(leaf);
    if (!editor->linkSchematic(fn))
      QMessageBox::warning(mw, "Failed to link schematic",
                           "Cannot link schematic “" + fn
                           + "”. Could the file be damaged?");
  } else if (mb.clickedButton()==browse) {
    linkSchematicDialog();
  }
}

void MWData::discussRelinkWhileLoading() {
  /* If our file got moved (i.e., loaded from a different location
     than the "pcbfilename" in Board), determine whether the
     difference affects the path and whether it affects the
     leaf. Depending on which parts differ, check for the existence of
     circuit files with updated path and/or leaf. Offer to load one of
     those instead.
     NYI
   */
}

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
    path += QFileInfo(filename).completeBaseName();
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
  openFiles().remove(filename);
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

void MWData::openLinkedSchematic() {
  QString fn = editor->linkedSchematicFilename();
  if (fn.isEmpty())
    return;
  QDesktopServices::openUrl(QUrl::fromLocalFile(fn));
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
    QObject::connect(&editor->linkedSchematic(), &LinkedSchematic::reloaded,
                     mcv, [this]() {
                       mcv->setSchem(editor->linkedSchematic().schematic());
                     });
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
    mw->addDockWidget(Qt::BottomDockWidgetArea, bomvdock);
  }
}

void MWData::newWindow() {
  auto *w = new MainWindow();
  w->resize(mw->size());
  w->show();
  // attempt to raise...
  //  w->raise();
  //  w->setFocus();
}

void MWData::revert() {
  if (!editor->isAsSaved()) {
    auto ret= QMessageBox::warning(mw, "cpcb",
			     mw->tr("The layout has been modified.\n"
				"Do you want to discard your changes?"),
                                   QMessageBox::Discard
                                   | QMessageBox::Cancel);
    if (ret!=QMessageBox::Discard)
      return;
  }
  mw->open(filename);
}

void MWData::openDialog() {
  QString fn = getOpenFilename("cpcb", "Select file to open…",
                               "PCB layouts (*.cpcb)");
  if (!fn.isEmpty()) {
    if (openFiles().contains(fn)) {
      openFiles()[fn]->show();
      openFiles()[fn]->raise();
    } else {
      auto *w = editor->pcbLayout().root().isEmpty() ? mw : new MainWindow();
      w->open(fn);
      w->show();
    }
  }
}

bool MainWindow::open(QString fn) {
  openFiles().remove(d->filename);
  QFileInfo fi(fn);
  openFiles()[fn] = this;
  fn = fi.absoluteFilePath();
  if (!d->editor->load(fn)) {
    d->resetFilename();
    QMessageBox::warning(this, "CPCB",
			 "Could not load “" + fn + "”",
			 QMessageBox::Ok);
    return false;
  }
  
  d->filename = fi.absoluteFilePath();
  d->recentfiles->mark(d->filename);
  d->pwd = fi.dir().absolutePath();
  setWindowTitle(d->filename);

  if (!d->editor->pcbLayout().board().linkedschematic.isEmpty()) {
    if (!d->editor->linkedSchematic().isValid())
      d->attemptRelinkSchematic();
    else if (d->filename !=  d->editor->pcbLayout().board().pcbfilename)
      d->discussRelinkWhileLoading();
  }
  if (d->editor->linkedSchematic().isValid()) 
    d->showParts();

  return true;
}  

bool MWData::saveImmediately() {
  if (filename.isEmpty()) {
    return saveAsDialog();
  } else {
    bool ok = editor->save(filename);
    if (ok)
      recentfiles->mark(filename);
    else
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
  QString gname = editor->selectedComponentGroup()
    .attributes.value(Group::Attribute::Footprint);
  QString sug = compwd;
  if (gname!="")
    sug += "/" + gname + ".svg";
  QString fn = QFileDialog::getSaveFileName(0, "Save component…",
					    sug,
					    "PCB components (*.svg)");
  if (fn.isEmpty())
    return;

  if (!fn.endsWith(".svg"))
    fn += ".svg";

  compwd = QFileInfo(fn).dir().absolutePath();
  
  editor->saveComponent(id, fn);
}

void MWData::openLibrary() {
  //qDebug() << QUrl::fromLocalFile(Paths::userComponentRoot());
  QDesktopServices::openUrl(QUrl::fromLocalFile(Paths::userComponentRoot()));
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
      && nm.missingEntirely.isEmpty()) {
    QMap<QString, QSet<QString>> duppins = grp.groupsWithDuplicatedPins();
    if (!duppins.isEmpty()) {
      auto it = duppins.begin();
      QString msg = "Part " + it.key() + " has duplicated pin names:";
      for (QString n: it.value())
        msg += "\n  " + n;
      msg += "\nApart from that, all nets verified OK.";
      QMessageBox::information(mw, "cpcb", msg);
    } else {
      QMessageBox::information(mw, "cpcb", "All nets verified OK.");
    }
  } else {
    QStringList msgs;
    msgs << "Verification unsuccessful:";
    QSet<QString> missing;
    for (NodeID const &n: nm.missingFromNet) 
      missing << grp.nodeName(n).humanName();
    QStringList missinglist;
    for (QString m: missing)
      missinglist << m;
    if (missinglist.size()==1) 
      msgs << missinglist[0] + " is missing a connection.";
    else if (missinglist.size()>1)
      msgs << "Some connections are missing, including from "
        + missinglist[0] + ".";

    QSet<QString> wrongly;
    for (NodeID const &n: nm.wronglyInNet) 
      wrongly << grp.nodeName(n).humanName();
    QStringList wronglylist;
    for (QString m: wrongly)
      wronglylist << m;
    if (wronglylist.size()==1)
      msgs << wronglylist[0] + " has a spurious connection.";
    else if (wronglylist.size()>1)
      msgs << "There are spurious connections, including on "
        + wronglylist[0] + ".";

    QStringList missingEntirely;
    for (Nodename const &n: nm.missingEntirely)
      missingEntirely << n.humanName();
    if (missingEntirely.size()==1)
      msgs << missingEntirely[0] + " has not been placed.";
    else if (missingEntirely.size()>1)
      msgs << "Some pins or components have not been placed.";
    msgs << "";
    if (nm.missingFromNet.size() || nm.wronglyInNet.size())
      msgs << "One affected net has been highlighted.";
    if (nm.missingEntirely.size()>1)
      msgs << "See status bar for details.";
    QMessageBox::warning(mw, "cpcb", msgs.join("\n"));
    return;
  }  
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
  double angle = QInputDialog::getDouble(mw, "Arbitrary rotation",
				   "Clockwise angle (degrees):",
				   0, -359.99, 359.99);
  if (angle)
    editor->arbitraryRotation(FreeRotation(angle));
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

bool MWData::exportDialog() {
  ExportDialog dlg(mw);
  if (pwd.isEmpty())
    pwd = Paths::defaultLocation();
  if (!dlg.runDialog(filename, pwd))
    return false;
  return dlg.saveAccordingly(editor->pcbLayout(), editor->linkedSchematic());
}

bool MWData::importBOMDialog() {
  QString fn = getOpenFilename("csv", "Import BOM…");
  if (fn.isEmpty())
    return false;
  QString error = editor->loadBOM(fn);
  if (error.isEmpty())
    return true;
  QMessageBox::warning(mw, "cpcb",
		       "Could not load BOM from “"
		       + fn + "”: " + error,
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

  openFiles().remove(filename);
  filename = fn;
  openFiles()[filename] = mw;

  pwd = QFileInfo(fn).absolutePath();
  mw->setWindowTitle(fn);
  recentfiles->mark(fn);

  discussRelinkWhileSaving();

  if (editor->save(fn))
    return true;

  QMessageBox::warning(mw, "cpcb",
		       "Could not save pcb as “"
		       + fn + "”",
		       QMessageBox::Ok);
  return false;
}  

void MWData::about() {
  QString me = "<b>CPCB</b>";
  QString vsn = Version::toString();
  QMessageBox::about(mw, "About CPCB",
		     me + " " + vsn
		     + "<p>" + "(C) 2018–2024 Daniel A. Wagenaar\n"
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
		   mcv, [this]() {
                     mcv->setRoot(editor->pcbLayout().root());
                   });
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
  QObject::connect(&editor->linkedSchematic(), &LinkedSchematic::reloaded,
                   bomv, [this]() {
                     bomv->model()->rebuild();
                   });
  QObject::connect(editor, &Editor::componentsChanged,
		   bomv, [this]() {
                     bomv->model()->rebuild();
                   });
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

  recentfiles = new RecentFiles("cpcb-recent", mw);
  mw->connect(recentfiles, &RecentFiles::selected,
	  [this](QString fn) {
            if (openFiles().contains(fn)) {
              openFiles()[fn]->show();
              openFiles()[fn]->raise();
            } else {
              auto *mw1 = editor->pcbLayout().root().isEmpty()
                ? mw : new MainWindow();
              mw1->open(fn);
              mw1->show();
            }
          });
  file->addMenu(recentfiles);
  
  a = file->addAction("&Save", [this]() { saveImmediately(); },
		      QKeySequence(Qt::CTRL + Qt::Key_S));
  QObject::connect(editor, &Editor::changedFromSaved,
		   a, &QAction::setEnabled);

  file->addAction("Save &as…", [this]() { saveAsDialog(); },
		  QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::Key_S));

  file->addAction("Re&vert", [this]() { revert(); });

  //  auto *fexport = file->addMenu("&Export");
  file->addAction("&Export fabrication files…",
                  [this]() { exportDialog(); });

  file->addAction("&Import BOM from CSV…",
                  [this]() { importBOMDialog(); });
  file->addAction("&Copy PCB image to clipboard",
                  [this]() { copyPCBImage(); },
		  QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::Key_I));
		  
  
  file->addAction("&Quit", []() { QApplication::quit(); });

  auto *edit = mb->addMenu("&Edit");

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
  
  a = edit->addAction("&Cut", [this]() { passthroughSignal("cut"); },
		      QKeySequence(Qt::CTRL + Qt::Key_X));
  QObject::connect(editor, &Editor::selectionChanged,
		   a, &QAction::setEnabled);
  a->setEnabled(false);
  
  a = edit->addAction("Cop&y", [this]() { passthroughSignal("copy"); },
		      QKeySequence(Qt::CTRL + Qt::Key_C));
  QObject::connect(editor, &Editor::selectionChanged,
		   a, &QAction::setEnabled);
  a->setEnabled(false);
  
  edit->addAction("&Paste", [this]() { passthroughSignal("paste"); },
		  QKeySequence(Qt::CTRL + Qt::Key_V));

  a = edit->addAction("&Delete selected",
		      [this]() { passthroughSignal("deleet"); },
		      QKeySequence(Qt::Key_Delete));
  QObject::connect(editor, &Editor::selectionChanged,
                   a, &QAction::setEnabled);
  a->setEnabled(false);
  
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

  auto *xforms = edit->addMenu("&Transform");

  a = xforms->addAction("&Rotate clockwise",
		      [this]() { editor->rotateCW(false, true); },
		      QKeySequence(Qt::CTRL + Qt::Key_R));
  QObject::connect(editor, &Editor::selectionChanged,
		   a, &QAction::setEnabled);
  a->setEnabled(false);

  a = xforms->addAction("Rotate clockwise (incl. text)",
		      [this]() { editor->rotateCW(); },
		      QKeySequence(Qt::CTRL + Qt::ALT + Qt::Key_R));
  QObject::connect(editor, &Editor::selectionChanged,
		   a, &QAction::setEnabled);
  a->setEnabled(false);

  a = xforms->addAction("Rotate &anticlockwise",
		      [this]() { editor->rotateCCW(false, true); },
		      QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::Key_R));
  QObject::connect(editor, &Editor::selectionChanged,
		   a, &QAction::setEnabled);
  a->setEnabled(false);

  a = xforms->addAction("Rotate &anticlockwise (incl. text)",
		      [this]() { editor->rotateCCW(); },
		      QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::ALT + Qt::Key_R));
  QObject::connect(editor, &Editor::selectionChanged,
		   a, &QAction::setEnabled);
  a->setEnabled(false);
  
  a = xforms->addAction("Arbitrary rotatio&n…",
		      [this]() { arbitraryRotation(); },
		      QKeySequence(Qt::ALT + Qt::Key_R));
  QObject::connect(editor, &Editor::selectionChanged,
		   a, &QAction::setEnabled);
  a->setEnabled(false);
    
  a = xforms->addAction("&Flip left–right", [this]() { editor->flipH(false, true); },
		      QKeySequence(Qt::CTRL + Qt::Key_F));
  QObject::connect(editor, &Editor::selectionChanged,
		   a, &QAction::setEnabled);
  a->setEnabled(false);
  
  a = xforms->addAction("Flip up–down", [this]() { editor->flipV(false, true); },
                  QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::Key_F));
  QObject::connect(editor, &Editor::selectionChanged,
		   a, &QAction::setEnabled);
  a->setEnabled(false);
  

  a = edit->addAction("Linear pattern",
                      [this]() { LinearPatternDialog::gui(editor,
                                   editor->pcbLayout().board()
                                     .isEffectivelyMetric(), mw); },
		      QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::Key_L));
  QObject::connect(editor, &Editor::selectionChanged,
		   a, &QAction::setEnabled);
  a->setEnabled(false);

  a = edit->addAction("Circular pattern",
                      [this]() { CircularPatternDialog::gui(editor,
                                     modebar->isOriginIncremental(), mw); },
		      QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::Key_C));
  QObject::connect(editor, &Editor::selectionChanged,
		   a, &QAction::setEnabled);
  a->setEnabled(false);

  a = edit->addAction("&Group", [this]() { editor->formGroup(); },
		      QKeySequence(Qt::CTRL + Qt::Key_G));
  QObject::connect(editor, &Editor::selectionChanged,
		   a, &QAction::setEnabled);
  a->setEnabled(false);
  
  a = edit->addAction("&Ungroup", [this]() { editor->dissolveGroup(); },
		      QKeySequence(Qt::CTRL + Qt::Key_U));
  QObject::connect(editor, &Editor::selectionIsGroup,
		   a, &QAction::setEnabled);
  a->setEnabled(false);

  a = edit->addAction("&Enter group", [this]() { editor->enterGroup(-1); });
  QObject::connect(editor, &Editor::selectionIsGroup,
		   a, &QAction::setEnabled);
  a->setEnabled(false);
  
  edit->addAction("Find", [this]() { Find(editor).run(); },
                  QKeySequence(Qt::Key_Slash));

  edit->addAction("Toggle &grid", [this]() { statusbar->toggleGrid(); },
                  QKeySequence("#"));
  edit->addAction("Cycle grid", [this]() { statusbar->nextGrid(); },
                  QKeySequence(Qt::Key_Period));
  edit->addAction("Reverse cycle grid",
                  [this]() { statusbar->previousGrid(); },
                  QKeySequence(Qt::Key_Comma));

  
  auto *tools = mb->addMenu("&Tools");
  tools->addAction("&Open library", [this]() { openLibrary(); },
		  QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::Key_Y));
  tools->addAction("&Insert component…", [this]() { insertComponentDialog(); },
		   QKeySequence(Qt::CTRL + Qt::Key_I));
  a = tools->addAction("&Save component…", [this]() { saveComponentDialog(); },
		      QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::Key_G));
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
  
  a = tools->addAction("&Link schematic…", [this]() { linkSchematicDialog(); },
		  QKeySequence(Qt::CTRL + Qt::Key_L));
  QObject::connect(editor, &Editor::schematicLinked,
		   a, &QAction::setDisabled);

  auto *links = tools->addMenu("Lin&ked schematic");
  QObject::connect(editor, &Editor::schematicLinked,
		   links, &QMenu::setEnabled);
  links->setEnabled(false);
  links->addAction("&Open",
                   [this]() { openLinkedSchematic(); });
  links->addAction("&Unlink",
                   [this]() { editor->unlinkSchematic(); },
                   QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::Key_U));
  auto *linkinfo = links->addMenu("&Info");

  linklabel = new QLabel(editor->pcbLayout().board().linkedschematic);
  linklabel->setAlignment(Qt::AlignCenter);
  linklabel->setMargin(4);
  QObject::connect(editor, &Editor::schematicLinked,
                   linklabel, [this]() {
                     linklabel->setText(editor->pcbLayout().board()
                                        .linkedschematic);
                   });
  QWidgetAction *wa = new QWidgetAction(linkinfo);
  wa->setDefaultWidget(linklabel);
  linkinfo->addAction(wa);
  
  
  a = tools->addAction("&Verify nets",
		      [this]() { verifyNets(); },
		      QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::Key_V));
  QObject::connect(editor, &Editor::schematicLinked,
		   a, &QAction::setEnabled);
  a->setEnabled(false);
  
  auto *view = mb->addMenu("&View");
  QAction *act = new QAction("&Scale to fit", mw);
  act->setShortcuts(QList<QKeySequence>()
                    << QKeySequence(Qt::CTRL + Qt::Key_0)
                    << QKeySequence(Qt::Key_0));
  mw->connect(act, &QAction::triggered, [this]() { editor->scaleToFit(); });
  view->addAction(act);
  
  act = new QAction("Zoom &in", mw);
  act->setShortcuts(QList<QKeySequence>()
                    << QKeySequence(QKeySequence::ZoomIn)
		    << QKeySequence(Qt::CTRL + Qt::Key_Equal)
		    << QKeySequence(Qt::Key_Equal)
		    << QKeySequence(Qt::Key_Plus));
  mw->connect(act, &QAction::triggered, [this]() { editor->zoomIn(); });
  view->addAction(act);

  act = new QAction("Zoom &out", mw);
  act->setShortcuts(QList<QKeySequence>()
                    << QKeySequence::ZoomOut
                    << QKeySequence(Qt::Key_Minus));
  mw->connect(act, &QAction::triggered, [this]() { editor->zoomOut(); });
  view->addAction(act);
  
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
  QObject::connect(editor, &Editor::escapePressed,
		   [this]() { modebar->setMode(Mode::Edit); });
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
		   statusbar, &Statusbar::setMode);
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
                       //qDebug() << "I might make the planes visible now";
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
  //qDebug() << "mw:fillbars";
  propbar->reflectSelection();
  propbar->forwardAllProperties();
}

MainWindow::MainWindow(): QMainWindow() {
  setAttribute(Qt::WA_DeleteOnClose);
  setWindowIcon(QIcon(":/cpcb.png"));
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
  QStringList ss;
  for (auto it=openFiles().begin(); it!=openFiles().end(); ++it) 
    if (it.value()==this)
      ss << it.key();
  for (QString s: ss)
    openFiles().remove(s);
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
  //qDebug() << "selection to bom";
  if (editor->breadcrumbs().size())
    return; // we only care at top level
  QSet<int> sel = editor->selectedObjects();
  bomv->selectElements(sel);
}

void MWData::selectionFromBOM() {
  //  qDebug() <<"selection from bom";
  if (editor->breadcrumbs().size())
    return; // we only care at top level
  if (bomv->isQuiet())
    return;
  QSet<int> sel = bomv->selectedElements();
  editor->select(sel, true);
}
