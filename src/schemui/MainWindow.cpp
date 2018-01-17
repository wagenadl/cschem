// MainWindow.cpp

#include "MainWindow.h"
#include <QGraphicsView>
#include "Scene.h"
#include "circuit/Schem.h"
#include "svg/SymbolLibrary.h"
#include "file/FileIO.h"
#include <QMenuBar>
#include <QApplication>
#include <QStatusBar>
#include <QDir>
#include <QFileDialog>
#include <QDebug>
#include "Style.h"
#include <QMessageBox>
#include "LibView.h"
#include "PartListView.h"
#include <QDockWidget>
#include "LibView.h"
#include "qt/SignalAccumulator.h"
#include "svg/SvgExporter.h"
#include  <QClipboard>

class MWData {
public:
  MWData():
    view(0),
    scene(0),
    libview(0), libviewdock(0),
    partlistview(0), partlistviewdock(0),
    unsaved(false) {
  }
public:
  SymbolLibrary lib;
  QGraphicsView *view;
  Scene *scene;
  Schem schem;
  QString filename;
  static QString lastdir;
  LibView *libview;
  QDockWidget *libviewdock;
  PartListView *partlistview;
  QDockWidget *partlistviewdock;
  SignalAccumulator *chgtoplv;
  bool unsaved;
};

QString MWData::lastdir;
    
MainWindow::MainWindow(SymbolLibrary const *lib): d(new MWData()) {
  if (lib)
    d->lib = *lib;
  else
    d->lib = SymbolLibrary::defaultSymbols();
  createView();
  createDocks();
  createActions();
  d->view->scale(1.5, 1.5);
  d->libview->scale(1.5);
  create();
  int w0 = 10 * d->libview->width();
  int h0 = 3 * w0 / 4;
  qDebug() << w0 << h0;
  int w = width();
  int h = height();
  if (w < w0)
    w = w0;
  if (h < h0)
    h = h0;
  resize(w, h);
  d->partlistviewdock->hide();
}

void MainWindow::createDocks() {
  d->libview = new LibView(&d->lib);
  connect(d->libview, SIGNAL(activated(QString)),
          this, SLOT(plonk(QString)));
  d->libviewdock = new QDockWidget("Library", this);
  d->libviewdock->setWidget(d->libview);
  showLibrary();

  d->partlistview = new PartListView(&d->schem);
  d->partlistviewdock = new QDockWidget("Part list", this);
  d->partlistviewdock->setWidget(d->partlistview);
  showSymbolsList();
  d->chgtoplv = new SignalAccumulator(this);
  connect(d->chgtoplv, &SignalAccumulator::activated,
          d->partlistview, &PartListView::rebuild);
  connect(d->partlistview, &PartListView::valueEdited,
          this, &MainWindow::reactToPartListEdit);
}

void MainWindow::showLibrary() {
  addDockWidget(Qt::LeftDockWidgetArea, d->libviewdock);
  d->libviewdock->show();
}


void MainWindow::showSymbolsList() {
  addDockWidget(Qt::RightDockWidgetArea, d->partlistviewdock);
  d->partlistview->rebuild();
  d->partlistviewdock->show();
}

void MainWindow::showVirtuals() {
}

void MainWindow::createView() {
  d->view = new QGraphicsView(this);
  d->view->setInteractive(true);
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
  connect(act, &QAction::triggered, this, &MainWindow::saveAction);
  menu->addAction(act);

  act = new QAction(tr("Save &as…"), this);
  act->setShortcuts(QKeySequence::SaveAs);
  connect(act, &QAction::triggered, this, &MainWindow::saveAsAction);
  menu->addAction(act);

  act = new QAction(tr("&Export circuit as svg…"), this);
  act->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_E));
  connect(act, &QAction::triggered, this, &MainWindow::exportCircuitAction);
  menu->addAction(act);

  act = new QAction(tr("Export &parts list as csv…"), this);
  act->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_L));
  connect(act, &QAction::triggered, this, &MainWindow::exportPartListAction);
  menu->addAction(act);

  act = new QAction(tr("Copy circuit &image to clipboard"), this);
  act->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_G));
  connect(act, &QAction::triggered, this, &MainWindow::circuitToClipboardAction);
  menu->addAction(act);

  act = new QAction(tr("Copy symbols lis&t to clipboard"), this);
  act->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_T));
  connect(act, &QAction::triggered,
	  this, &MainWindow::partListToClipboardAction);
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

  act = new QAction(tr("Re&do"), this);
  act->setShortcuts(QKeySequence::Redo);
  act->setStatusTip(tr("Redo"));
  connect(act, &QAction::triggered, this, &MainWindow::redoAction);
  menu->addAction(act);

  act = new QAction(tr("Rotate &right"), this);
  act->setShortcuts(QList<QKeySequence>()
                   << QKeySequence(Qt::SHIFT + Qt::Key_R)
                   << QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::Key_R));
  act->setStatusTip(tr("Rotate clockwise"));
  connect(act, &QAction::triggered, this, &MainWindow::rotateCWAction);
  menu->addAction(act);

  act = new QAction(tr("Rotate &left"), this);
  act->setShortcuts(QList<QKeySequence>()
                   << QKeySequence(Qt::Key_R)
                   << QKeySequence(Qt::CTRL + Qt::Key_R));
  act->setStatusTip(tr("Rotate clockwise"));
  connect(act, &QAction::triggered, this, &MainWindow::rotateCCWAction);
  menu->addAction(act);

  act = new QAction(tr("Flip"), this);
  act->setShortcuts(QList<QKeySequence>()
                   << QKeySequence(Qt::Key_F)
                   << QKeySequence(Qt::CTRL + Qt::Key_F));
  act->setStatusTip(tr("Flip horizontally"));
  connect(act, &QAction::triggered, this, &MainWindow::flipAction);
  menu->addAction(act);

  menu = menuBar()->addMenu(tr("&Tools"));
  
  act = new QAction(tr("Remove &dangling connections"), this);
  act->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_B));
  act->setStatusTip(tr("Cleanup circuit by removing dangling connections"));
  connect(act, &QAction::triggered, this, &MainWindow::removeDanglingAction);
  menu->addAction(act);

  menu = menuBar()->addMenu(tr("&View"));

  act = new QAction(tr("&Zoom in"), this);
  act->setShortcuts(QList<QKeySequence>()
                    << QKeySequence(QKeySequence::ZoomIn)
		    << QKeySequence(Qt::CTRL + Qt::Key_Equal)
		    << QKeySequence(Qt::Key_Equal)
		    << QKeySequence(Qt::Key_Plus));
  act->setStatusTip(tr("Zoom in"));
  connect(act, &QAction::triggered, this, &MainWindow::zoomIn);
  menu->addAction(act);

  act = new QAction(tr("&Zoom out"), this);
  act->setShortcuts(QList<QKeySequence>()
                    << QKeySequence::ZoomOut
                    << QKeySequence(Qt::Key_Minus));
  act->setStatusTip(tr("Zoom out"));
  connect(act, &QAction::triggered, this, &MainWindow::zoomOut);
  menu->addAction(act);

  act = new QAction(tr("&Library"), this);
  act->setStatusTip(tr("Show library pane"));
  connect(act, &QAction::triggered, this, &MainWindow::showLibrary);
  menu->addAction(act);


  act = new QAction(tr("&Parts List"), this);
  act->setStatusTip(tr("Show parts list"));
  connect(act, &QAction::triggered, this, &MainWindow::showSymbolsList);
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

bool MainWindow::saveAction() {
  if (d->filename.isEmpty())
    return saveAsAction();
  else
    return saveAs(d->filename);
}

bool MainWindow::saveAsAction() {
  if (d->lastdir.isEmpty())
    d->lastdir = QDir::home().absoluteFilePath("Desktop");
  QFileDialog dlg;
  dlg.setWindowTitle(tr("Save schematic as…"));
  dlg.setAcceptMode(QFileDialog::AcceptSave);
  dlg.setDefaultSuffix("schem");
  dlg.setDirectory(d->lastdir);
  dlg.setNameFilter(tr("Schematics (*.schem)"));
  if (!d->filename.isEmpty()) {
    QFileInfo fi(d->filename);
    dlg.selectFile(fi.baseName() + ".schem");
  }
  if (!dlg.exec())
    return false;
  QStringList fns = dlg.selectedFiles();
  if (fns.isEmpty())
    return false;

  d->lastdir = dlg.directory().absolutePath();
  
  return saveAs(fns.first());
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

  connect(d->scene, &Scene::annotationEdited,
          this, &MainWindow::reactToSceneEdit);
  connect(d->scene, SIGNAL(libraryChanged()),
	  d->libview, SLOT(rebuild()));
}

void MainWindow::load(QString fn) {
  create();
  qDebug() << "Created schematic";
  d->schem = FileIO::loadSchematic(fn);
  qDebug() << "loaded schematic. symbol names:" << d->schem.library().symbolNames();
  d->libview->clear();
  for (QString name: d->schem.library().symbolNames())
    d->lib.insert(d->schem.library().symbol(name));
  d->scene->setCircuit(d->schem.circuit());
  d->partlistview->rebuild();
  d->libview->rebuild();
  setWindowTitle(fn);
  d->filename = fn;
}

bool MainWindow::saveAs(QString fn) {
  Circuit circ(d->scene->circuit());
  circ.renumber(1);
  d->schem.setCircuit(circ);
  d->schem.selectivelyUpdateLibrary(d->lib);
  if (FileIO::saveSchematic(fn, d->schem)) {
    setWindowTitle(fn);
    d->filename = fn;
    d->unsaved = false;
    return true;
  } else {
    QMessageBox::warning(this, Style::programName(),
			 tr("Could not save schematic as “")
			 + fn + tr("”"),
			 QMessageBox::Ok);
    return false;
  }
}

void MainWindow::markChanged() {
  setWindowTitle(d->filename + " *");
}

void MainWindow::zoomIn() {
  d->view->scale(1.5, 1.5);
}

void MainWindow::zoomOut() {
  d->view->scale(1/1.5, 1/1.5);
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

void MainWindow::reactToSceneEdit() {
  d->schem.setCircuit(d->scene->circuit());
  if (d->partlistview->isVisible())
    d->chgtoplv->activate();
  d->unsaved = true;
  setWindowTitle("*" + (d->filename.isEmpty() ? "Untitled" : d->filename));
}

void MainWindow::closeEvent(QCloseEvent *e) {
  if (d->unsaved) {
    auto ret
      = QMessageBox::warning(this, Style::programName(),
			     tr("The schematic has been modified.\n"
				"Do you want to save your changes?"),
			     QMessageBox::Save
			     | QMessageBox::Discard
			     | QMessageBox::Cancel);
    switch (ret) {
    case QMessageBox::Save:
      if (saveAction())
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
  } else {
    e->accept();
  }
}

void MainWindow::reactToPartListEdit(int id) {
  d->scene->setComponentValue(id, d->schem.circuit().element(id).value());
}

void MainWindow::resizeEvent(QResizeEvent *e) {
  QMainWindow::resizeEvent(e);
  d->partlistview->resetWidth();
}

void MainWindow::flipAction() {
  d->scene->flipx();
}

void MainWindow::rotateCCWAction() {
  d->scene->rotate(1);
}

void MainWindow::rotateCWAction() {
  d->scene->rotate(-1);
}

void MainWindow::exportCircuitAction() {
  d->schem.setCircuit(d->scene->circuit());
  
  if (d->lastdir.isEmpty())
    d->lastdir = QDir::home().absoluteFilePath("Desktop");
  
  QFileDialog dlg;
  dlg.setWindowTitle(tr("Export schematic as svg…"));
  dlg.setAcceptMode(QFileDialog::AcceptSave);
  dlg.setDefaultSuffix("schem");
  dlg.setDirectory(d->lastdir);
  //dlg.setFilter(QDir::AllFiles);
  dlg.setNameFilter(tr("Scalable vector graphics (*.svg)"));
  if (!d->filename.isEmpty()) {
    QFileInfo fi(d->filename);
    dlg.selectFile(fi.baseName() + ".svg");
  }
  if (!dlg.exec())
    return;
  QStringList fns = dlg.selectedFiles();
  if (fns.isEmpty())
    return;

  d->lastdir = dlg.directory().absolutePath();

  QString fn = fns.first();

  SvgExporter xp(d->schem.circuit(),&d->lib);
  if (!xp.exportSvg(fn)) {
    qDebug() << "Failed to export svg";
  }
}

void MainWindow::exportPartListAction() {
  if (d->lastdir.isEmpty())
    d->lastdir = QDir::home().absoluteFilePath("Desktop");
  
  QFileDialog dlg;
  dlg.setWindowTitle(tr("Export parts list as csv…"));
  dlg.setAcceptMode(QFileDialog::AcceptSave);
  dlg.setDefaultSuffix("schem");
  dlg.setDirectory(d->lastdir);
  //dlg.setFilter(QDir::AllFiles);
  dlg.setNameFilter(tr("Comma-separated value file (*.csv)"));
  if (!d->filename.isEmpty()) {
    QFileInfo fi(d->filename);
    dlg.selectFile(fi.baseName() + ".csv");
  }
  if (!dlg.exec())
    return;
  QStringList fns = dlg.selectedFiles();
  if (fns.isEmpty())
    return;

  d->lastdir = dlg.directory().absolutePath();

  QString fn = fns.first();

  QFile file(fn);
  if (file.open(QFile::WriteOnly)) {
    QTextStream ts(&file);
    for (QStringList  line: d->partlistview->partList()) {
      for (QString &str: line) {
	str.replace("\"", tr("”"));
	str = "\"" + str + "\"";
      }
      ts << line.join(",");
      ts << "\n";
    }
  } else {
    qDebug() << "Failed to export part list";
    // should show error box
  }
}

void MainWindow::circuitToClipboardAction() {
  QRectF rr = d->scene->itemsBoundingRect();
  QSizeF ss = rr.size();
  QSizeF sdest = 2*ss;
  QRectF rdest = QRectF(QPointF(), sdest);
  QPixmap img(sdest.toSize());
  img.fill();
  { QPainter ptr(&img);
    d->scene->unhover();
    d->scene->render(&ptr, rdest, rr);
    d->scene->rehover();
  }
  QApplication::clipboard()->setPixmap(img);
}
 
void MainWindow::partListToClipboardAction() {
  QList<QStringList> symbols = d->partlistview->partList();
  QString text;
  for (QStringList const &line: symbols)
    text += line.join("\t") + "\n";
  QApplication::clipboard()->setText(text);
  
} 
