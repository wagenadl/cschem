// MainWindow.h

#ifndef MAINWINDOW_H

#define MAINWINDOW_H

#include <QMainWindow>

class MainWindow: public QMainWindow {
public:
  MainWindow();
  virtual ~MainWindow();
private:
  class MWData *d;
};

#endif
