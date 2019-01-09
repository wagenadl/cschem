// Paths.cpp

#include "Paths.h"
#include <QDir>

namespace Paths {
  QString userComponentRoot() {
    return QDir::home().absolutePath() + "/.local/cschem/outlines";
  }
  QString recentSymbolsLocation() {
    return QDir::home().absolutePath() + "/.local/cschem/cpcb-recent";
  }
  QString defaultLocation() {
    return QDir::home().absoluteFilePath("Desktop");
  }
  QString systemComponentRoot() {
    // of course, this will have to change for windows and perhaps mac
    QDir lcl("/usr/local/share/cschem/outlines");
    QDir usr("/usr/share/cschem/outlines");
    if (lcl.exists())
      return lcl.absolutePath();
    else if (usr.exists())
      return usr.absolutePath();
    else
      return "";
  }
};
