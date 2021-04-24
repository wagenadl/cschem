// Paths.h

#ifndef PATHS_H

#define PATHS_H

#include <QString>

namespace Paths {
  QString userSymbolRoot();
  QString systemSymbolRoot();
  QString defaultLocation();
  void setExecutablePath(QString);
};

#endif
