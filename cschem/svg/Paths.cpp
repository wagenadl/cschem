// Paths.cpp

#include "Paths.h"
#include <QDir>

namespace Paths {
  QString userSymbolRoot() {
    return QDir::home().absolutePath() + "/.local/cschem/symbols";
  }
  QString systemSymbolRoot() {
    // this should be changed for windows and perhaps even mac
    QDir lcl("/usr/local/share/cschem/symbols");
    QDir usr("/usr/share/cschem/symbols");
    if (lcl.exists())
      return lcl.absolutePath();
    else if (usr.exists())
      return usr.absolutePath();
    else
      return "";
  }
  QString defaultLocation() {
    return QDir::home().absoluteFilePath("Desktop");
  }
};
