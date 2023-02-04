// Paths.cpp

#include "Paths.h"
#include <QDir>
#include <QFileInfo>
#include <QStandardPaths>
#include <QDebug>

namespace Paths {
  static QDir installPath;

  void setExecutablePath(QString s) {
    QFileInfo exe(s);
    //qDebug() << "exe" << exe;
    QDir dir = exe.dir();
    dir.makeAbsolute();
    qDebug() << "dir" << dir;
    dir.cdUp();
    if (dir.path().endsWith("build"))
      dir.cdUp();
    installPath = dir;
    qDebug() << "dir" << installPath;
  }

  QString userComponentRoot() {
    QString root
      = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    return QDir(root).absoluteFilePath("pcb-outlines");
  }
  
  QString recentSymbolsLocation() {
  QString root
    = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
  return QDir(root).absoluteFilePath("cpcb-recent");
  }
  
  QString defaultLocation() {
    return QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);
  }

  QString systemComponentRoot() {
    QString userroot
      = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QStringList allroots
      = QStandardPaths::standardLocations(QStandardPaths::DataLocation);
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
  
