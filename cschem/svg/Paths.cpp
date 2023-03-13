// Paths.cpp

#include "Paths.h"
#include <QDir>
#include <QStandardPaths>
#include <QFileInfo>
#include <QDebug>

namespace Paths {
  static QDir installPath;

  void setExecutablePath(QString s) {
    QFileInfo exe(s);
    QDir dir = exe.dir();
    dir.makeAbsolute();
    qDebug() << "dir" << dir;
    dir.cdUp();
    if (dir.path().endsWith("build"))
      dir.cdUp();
    installPath = dir;
    qDebug() << "dir" << installPath;
  }

  QString userSymbolRoot() {
    QString root
      = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    return QDir(root).absoluteFilePath("symbols");
  }

  QString systemSymbolRoot() {
    QString userroot
      = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QStringList allroots
      = QStandardPaths::standardLocations(QStandardPaths::AppDataLocation);
    for (QString const &root: allroots) {
      if (root==userroot)
        continue;
      qDebug() << "allroots" << root;
      if (QDir(root).exists("symbols"))
        return QDir(root).absoluteFilePath("symbols");
    }
    if (QDir(installPath).exists("symbols"))
      return installPath.absoluteFilePath("symbols");
    else
      return QString();
  }

  QString defaultLocation() {
    return QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);
  }
};
