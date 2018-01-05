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
  QSharedPointer<class Scene> scene() const;
  void setScene(QSharedPointer<Scene> const &);
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
public:
  void load(QString filename);
  void create();
  void saveAs(QString filename);
private:
  void createActions();
  void createView();
  void createStatusBar();
private:
  class MWData *d;
};

#endif
