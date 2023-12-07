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
  current = fn;
  updateItems();
}

QStringList RecentFiles::list() const {
  return QSettings().value(varname).toStringList();
}

void RecentFiles::updateItems() {
  QStringList files = list();
  files.removeAll(current);
  QSet<QString> leaves;
  QSet<QString> dups;
  for (int n=0; n<MAXFILES; n++) {
    if (n<files.size()) {
      QString leaf = QFileInfo(files[n]).fileName();
      if (leaves.contains(leaf))
        dups << leaf;
      else
        leaves << leaf;
    }
  }
  for (int n=0; n<MAXFILES; n++) {
    if (n<files.size()) {
      QFileInfo fi(files[n]);
      QString leaf = fi.fileName();
      QString fn = dups.contains(leaf) ? fi.canonicalFilePath() : leaf; 
      QString text = tr("&%1 %2").arg(n + 1).arg(fn);
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

void RecentFiles::showEvent(QShowEvent *) {
  updateItems();
}
