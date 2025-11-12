// SvgExporter.h

#ifndef SVGEXPORTER_H

#define SVGEXPORTER_H

#include "data/Dim.h"
#include <QString>

class SvgExporter {
public:
  SvgExporter();
  bool write(class Layout const &layout, QString filename);
};

#endif
