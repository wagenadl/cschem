// RecentFiles.cpp

#include "RecentFiles.h"
#include <QDebug>
#include <QSettings>
#include <QFileInfo>

RecentFiles::RecentFiles(QString varname, QWidget *parent):
  QMenu("Open &recent", parent),
  varname(varname) {
  for (int n=0; n<MAXFILES; n++) {
    actions[n] = new QAction(this);
    actions[n]->setVisible(false);
    connect(actions[n], &QAction::triggered,
	    [this, n]() {
	      QString fn = actions[n]->data().toString();
	      if (!fn.isEmpty())
		emit selected(fn);
	      qDebug() << "recentfiles" << fn;
	    });
    addAction(actions[n]);
  }
  updateItems();
}

void RecentFiles::mark(QString fn) {
  QStringList files = list();
  files.removeAll(fn);
  files.prepend(fn);
  while (files.size() > MAXFILES)
    files.removeLast();
  QSettings().setValue(varname, files);
  updateItems();
}

QStringList RecentFiles::list() const {
  return QSettings().value(varname).toStringList();
}

void RecentFiles::updateItems() {
  QStringList files = list();
  for (int n=0; n<MAXFILES; n++) {
    if (n<files.size()) {
      QString leaf = QFileInfo(files[n]).fileName();
      QString text = tr("&%1 %2").arg(n + 1).arg(leaf);
      actions[n]->setText(text);
      actions[n]->setData(files[n]);
      actions[n]->setVisible(true);
    } else {
      actions[n]->setText("");
      actions[n]->setData(QString());
      actions[n]->setVisible(false);
    }
  }
}
