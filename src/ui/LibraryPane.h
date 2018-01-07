// LibraryPane.h

#ifndef LIBRARYPANE_H

#define LIBRARYPANE_H

#include <QListWidget>

class LibraryPane: public QListWidget {
  Q_OBJECT;
public:
  explicit LibraryPane(QWidget *parent=0);
  explicit LibraryPane(class PartLibrary const *lib, QWidget *parent=0);
  void rebuild(class PartLibrary const *lib);
  ~LibraryPane();
signals:
  void activated(QString);
private slots:
  void activateHelper(class QListWidgetItem *);
private:
  class LibraryPaneData *d;
};

#endif
