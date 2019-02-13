// FrontPanelWriter.h

#ifndef FRONTPANELWRITER_H

#define FRONTPANELWRITER_H

#include "data/Dim.h"
#include <QString>

class FrontPanelWriter {
public:
  FrontPanelWriter();
  ~FrontPanelWriter();
  bool write(class Layout const &layout, QString filename);
private:
  FrontPanelWriter(FrontPanelWriter const &) = delete;
  FrontPanelWriter &operator=(FrontPanelWriter const &) = delete;
  class FPWData *d;
};

#endif
