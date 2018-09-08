// MainWindow.h

#ifndef MAINWINDOW_H

#define MAINWINDOW_H

#include <QMainWindow>

class MainWindow: public QMainWindow {
public:
  MainWindow();
  virtual ~MainWindow();
  void open(QString);
protected:
  void closeEvent(QCloseEvent *);
private:
  class MWData *d;
  friend class MWData;
};

#endif
