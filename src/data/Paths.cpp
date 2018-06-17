// Paths.cpp

#include "Paths.h"
#include <QDir>

namespace Paths {
  QString componentRoot() {
    return QDir::home().absolutePath() + "/.local/cpcb/library";
  }
  QString defaultLocation() {
    return QDir::home().absoluteFilePath("Desktop");
  }
};
