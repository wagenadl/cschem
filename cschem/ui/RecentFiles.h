// RecentFiles.h

#ifndef RECENTFILES_H

#define RECENTFILES_H

#include <QString>
#include <QMenu>

class RecentFiles: public QMenu {
  Q_OBJECT;
public:
  static const int MAXFILES = 9;
  RecentFiles(QString varname="recentfiles", QWidget *parent=0);
  void mark(QString fn); // add to top of list, but exclude from showing
  QStringList list() const;
  void showEvent(QShowEvent *) override;
signals:
  void selected(QString);
private:
  void updateItems();
private:
  QString varname;
  QAction *actions[MAXFILES];
  QString current; // to be excluded
};

#endif
