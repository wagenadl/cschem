// MainWindow.h

#ifndef MAINWINDOW_H

#define MAINWINDOW_H

#include <QMainWindow>
#include <QSharedPointer>

class MainWindow: public QMainWindow {
  Q_OBJECT;
public:
  explicit MainWindow(class PartLibrary const *lib=0);
  ~MainWindow();
  MainWindow(MainWindow const &) = delete;
  MainWindow operator=(MainWindow const &) = delete;
  QSharedPointer<class Scene> scene() const;
  void setScene(QSharedPointer<Scene> const &);
public slots:
  void load(QString filename);
  void create();
  void saveAs(QString filename);
  void zoomIn();
  void zoomOut();
private:
  class MWData *d;
};

#endif
