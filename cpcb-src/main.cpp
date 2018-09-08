// main.cpp

#include "data/Trace.h"
#include "ui/MainWindow.h"
#include "data/Object.h"
#include <QApplication>
#include <QFile>
#include <QScreen>

int main(int argc, char **argv) {
  QApplication app(argc, argv);
  QStringList args = app.arguments();
  
  MainWindow mw;

  if (args.size()>=2)
    mw.open(args.last());  
  QSize avg = app.primaryScreen()->availableSize();
  mw.resize(3*avg.width()/4, 3*avg.height()/4);
  mw.show();
  return app.exec();
}

