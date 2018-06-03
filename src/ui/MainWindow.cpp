// MainWindow.cpp

#include "MainWindow.h"
#include "Modebar.h"
#include "Propertiesbar.h"
#include "Statusbar.h"
#include "Editor.h"

#include <QMessageBox>
#include <QMenu>
#include <QMenuBar>
#include <QApplication>

class MWData {
public:
  MWData(MainWindow *mw): mw(mw) {
  }
  void about();
  void makeToolbars();
  void makeMenus();
  void makeEditor();
  void makeConnections();
  void fillBars();
public:
  MainWindow *mw;
  Modebar *modebar;
  Propertiesbar *propbar;
  Statusbar *statusbar;
  Editor *editor;
};

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


void MWData::makeToolbars() {
  mw->addToolBar(Qt::LeftToolBarArea,
		 modebar = new Modebar(mw));
  mw->addToolBar(Qt::RightToolBarArea,
		 propbar = new Propertiesbar(mw));
  statusbar = new Statusbar(mw);
  mw->setStatusBar(statusbar);
}

void MWData::makeMenus() {
  auto *mb = mw->menuBar();

  auto *file = mb->addMenu("File");
  file->addAction("&Quit", []() { QApplication::quit(); });

  auto *view = mb->addMenu("View");
  view->addAction("&Scale to fit", [this]() { editor->scaleToFit(); },
		  QKeySequence(Qt::Key_F5));
  
  auto *help = mb->addMenu("Help");
  help->addAction("&About", [this]() { about(); });
}

void MWData::makeEditor() {
  editor = new Editor(mw);
  mw->setCentralWidget(editor);
}

void MWData::makeConnections() {
  QObject::connect(editor, &Editor::hovering,
		   statusbar, &Statusbar::setCursorXY);
  QObject::connect(editor, &Editor::leaving,
		   statusbar, &Statusbar::hideCursorXY);
}

void MWData::fillBars() {
  statusbar->setBoard(editor->pcbLayout().board());
}

MainWindow::MainWindow(): QMainWindow() {
  d = new MWData(this);
  d->makeToolbars();
  d->makeMenus();
  d->makeEditor();
  d->makeConnections();
  d->fillBars();
}

MainWindow::~MainWindow() {
  delete d;
}
