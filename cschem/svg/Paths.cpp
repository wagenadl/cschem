// Paths.cpp

#include "Paths.h"
#include <QDir>
#include <QStandardPaths>
#include <QFileInfo>
#include <QDebug>
#include <QCoreApplication>

namespace Paths {
  static QDir installPath;

  void setExecutablePath(QString s) {
    //QFileInfo exe(s);
    //QDir dir = exe.dir();
    QString appdir = QCoreApplication::applicationDirPath();
    QDir dir(appdir);
    dir.makeAbsolute();
    //    qDebug() << "dir" << dir;
    //dir.cdUp();
    if (dir.path().endsWith("build"))
      dir.cdUp();
    installPath = dir;
    //qDebug() << "installpath" << installPath;
  }

  QString userSymbolRoot() {
    QString root
      = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    //qDebug() << "usersymbolroot" << root << "symbols";
    return QDir(root).absoluteFilePath("symbols");
  }

  QString systemSymbolRoot() {
    QString userroot
      = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QStringList allroots
      = QStandardPaths::standardLocations(QStandardPaths::AppDataLocation);
    //qDebug() << "systemsymbolroot" << allroots << installPath;
    for (QString const &root: allroots) {
      if (root==userroot)
        continue;
      //qDebug() << "allroots" << root;
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
