// Paths.cpp

#include "Paths.h"
#include <QDir>
#include <QFileInfo>
#include <QStandardPaths>
#include <QDebug>
#include <QCoreApplication>

namespace Paths {
  static QDir installPath;

  void setExecutablePath(QString /*s*/) {
    QString appdir = QCoreApplication::applicationDirPath();
    //    QFileInfo exe(s);
    // qDebug() << "exe" << exe << exe.isAbsolute();
    //QDir dir = exe.dir();
    QDir dir(appdir);
    dir.makeAbsolute();
    //qDebug() << "dir" << dir;
    //dir.cdUp();
    if (dir.path().endsWith("build") || dir.path().endsWith("bin"))
      dir.cdUp();
    installPath = dir;
    //qDebug() << "installpath" << installPath;
  }

  QString userComponentRoot() {
    QString root
      = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    return QDir(root).absoluteFilePath("pcb-outlines");
  }
  
  QString recentSymbolsLocation() {
  QString root
    = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
  return QDir(root).absoluteFilePath("recent-symbol-instances");
  }
  
  QString defaultLocation() {
    return QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);
  }

  QString systemComponentRoot() {
    QString userroot
      = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QStringList allroots
      = QStandardPaths::standardLocations(QStandardPaths::AppDataLocation);
    qDebug() << "allroots = " << allroots;
    for (QString const &root: allroots) {
      if (root==userroot)
        continue;
      if (QDir(root).exists("pcb-outlines"))
        return QDir(root).absoluteFilePath("pcb-outlines");
    }
    qDebug() << "installpath = " << installPath;
    if (installPath.exists("pcb-outlines"))
      return installPath.absoluteFilePath("pcb-outlines");
    else
      return QString();
  }
};
  
