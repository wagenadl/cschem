// MainWindow.h

#ifndef MAINWINDOW_H

#define MAINWINDOW_H

#include <QMainWindow>
#include <QSharedPointer>

class MainWindow: public QMainWindow {
  Q_OBJECT;
public:
  explicit MainWindow(class PartLibrary const *lib=0);
  virtual ~MainWindow();
  MainWindow(MainWindow const &) = delete;
  MainWindow operator=(MainWindow const &) = delete;
public slots:
  void openAction();
  void saveAction();
  void saveAsAction();
  void newAction();
  void quitAction();
  void zoomIn();
  void zoomOut();
  void markChanged();
  void setStatusMessage(QString);
  void aboutAction();
  void copyAction();
  void cutAction();
  void pasteAction();
  void undoAction();
  void redoAction();
  void removeDanglingAction();
  void showLibrary();
  void showPartsList();
  void showVirtuals();
public:
  void load(QString filename);
  void create();
  void saveAs(QString filename);
private:
  void createActions();
  void createView();
  void createStatusBar();
  void createDocks();
private slots:
  void plonk(QString);
  void reactToSceneEdit();
private:
  class MWData *d;
};

#endif
