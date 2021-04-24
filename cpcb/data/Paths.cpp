// Paths.cpp

#include "Paths.h"
#include <QDir>
#include <QFileInfo>
#include <QStandardPaths>

namespace Paths {
  static QDir installPath;

  void setExecutablePath(QString s) {
    QFileInfo exe(s);
    installPath = exe.dir();
    installPath.cdUp();
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
    for (QString const &root: allroots) {
      if (root==userroot)
        continue;
      if (QDir(root).exists("pcb-outlines"))
        return QDir(root).absoluteFilePath("pcb-outlines");
    }
    if (QDir(installPath).exists("pcb-outlines"))
      return installPath.absoluteFilePath("pcb-outlines");
    else
      return QString();
  }
};
  
