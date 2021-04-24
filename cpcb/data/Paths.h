// Paths.h

#ifndef PATHS_H

#define PATHS_H

#include <QString>

namespace Paths {
  QString userComponentRoot();
  QString systemComponentRoot();
  QString recentSymbolsLocation();
  QString defaultLocation();
  void setExecutablePath(QString);
};

#endif
