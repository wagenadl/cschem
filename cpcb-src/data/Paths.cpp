// Paths.cpp

#include "Paths.h"
#include <QDir>

namespace Paths {
  QString componentRoot() {
    return QDir::home().absolutePath() + "/.local/cpcb/library";
  }
  QString recentSymbolsLocation() {
    return QDir::home().absolutePath() + "/.local/cpcb/recent";
  }
  QString defaultLocation() {
    return QDir::home().absoluteFilePath("Desktop");
  }
};
