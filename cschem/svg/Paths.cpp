// Paths.cpp

#include "Paths.h"
#include <QDir>

namespace Paths {
  QString symbolRoot() {
    return QDir::home().absolutePath() + "/.local/cschem/symbols";
  }
  QString defaultLocation() {
    return QDir::home().absoluteFilePath("Desktop");
  }
};
