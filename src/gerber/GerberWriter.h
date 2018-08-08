// GerberWriter.h

#ifndef GERBERWRITER_H

#define GERBERWRITER_H

#include "data/Layout.h"
#include "Gerber.h"
#include "data/Layer.h"
#include "Collector.h"

class GerberWriter {
public:
  GerberWriter(Layout const &layout, QString outputdir);
  ~GerberWriter();
  bool prepareFolder();
  bool writeLayer(Gerber::Layer layer);
public:
  static bool write(Layout const &layout, QString outputdir);
private:
  class GWData *d;
};

#endif
