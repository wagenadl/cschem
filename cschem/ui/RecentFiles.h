// RecentFiles.h

#ifndef RECENTFILES_H

#define RECENTFILES_H

#include <QString>
#include <QMenu>

class RecentFiles: public QMenu {
  Q_OBJECT;
public:
  static const int MAXFILES = 9;
  RecentFiles(QWidget *parent=0);
  void mark(QString fn);
  QStringList list() const;
signals:
  void selected(QString);
private:
  void updateItems();
private:
  QAction *actions[MAXFILES];
};

#endif
