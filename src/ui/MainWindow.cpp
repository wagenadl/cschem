// MainWindow.cpp

#include "MainWindow.h"
#include "Modebar.h"
#include "Propertiesbar.h"
#include "Statusbar.h"
#include <QMessageBox>
#include <QMenu>
#include <QMenuBar>
#include <QApplication>

class MWData {
public:
  MWData(MainWindow *mw): mw(mw) {
  }
  void about() {
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
public:
  MainWindow *mw;
  Modebar *modebar;
  Propertiesbar *propbar;
  Statusbar *statusbar;
};

MainWindow::MainWindow(): QMainWindow() {
  d = new MWData(this);
  addToolBar(Qt::LeftToolBarArea,
	     d->modebar = new Modebar(this));
  addToolBar(Qt::RightToolBarArea,
	     d->propbar = new Propertiesbar(this));
  d->statusbar = new Statusbar(this);
  setStatusBar(d->statusbar);

  auto *mb = menuBar();

  auto *file = mb->addMenu("File");
  file->addAction("Quit", []() { QApplication::quit(); });

  auto *help = mb->addMenu("Help");
  help->addAction("About", [this]() { d->about(); });

}

MainWindow::~MainWindow() {
  delete d;
}
