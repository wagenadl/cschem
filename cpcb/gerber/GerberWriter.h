// GerberWriter.h

#ifndef GERBERWRITER_H

#define GERBERWRITER_H

#include "data/Layout.h"
#include "Gerber.h"

class GerberWriter {
public:
  GerberWriter(Layout const &layout, QString outputdir);
  ~GerberWriter();
  bool prepareFolder();
  bool writeLayer(Gerber::Layer layer);
  bool writeAllLayers();
public:
  static bool write(Layout const &layout, QString outputdir);
private:
  class GWData *d;
};

#endif
