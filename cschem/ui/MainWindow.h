// MainWindow.h

#ifndef MAINWINDOW_H

#define MAINWINDOW_H

#include <QMainWindow>
#include <QSharedPointer>

class MainWindow: public QMainWindow {
  Q_OBJECT;
public:
  MainWindow();
  virtual ~MainWindow();
  MainWindow(MainWindow const &) = delete;
  MainWindow operator=(MainWindow const &) = delete;
public slots:
  void openAction();
  bool saveAction(); // true if actually saved
  bool saveAsAction(); // true if actually saved
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
  void openSymbolLibraryFolder();
  void rotateCCWAction();
  void rotateCWAction();
  void flipAction();
  void printPreviewAction();
  void printDialogAction();
  void exportCircuitAction();
  void exportPartListAction();
  void circuitImageToClipboardAction();
  void partListToClipboardAction();
  void compressedPartListToClipboardAction();
  void resolveConflictsAction();
public:
  bool load(QString filename); // true unless error
  void create(class Schem const &schem);
  bool saveAs(QString filename); // true unless error
private:
  void createActions();
  void createView();
  void createStatusBar();
  void createDocks();
private slots:
  void plonk(QString);
  void selectionToPartList();
  void selectionFromPartList();
protected:
  void resizeEvent(QResizeEvent *) override;
  void closeEvent(QCloseEvent *) override;
private:
  class MWData *d;
};

#endif
