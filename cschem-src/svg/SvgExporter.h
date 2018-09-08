// SvgExporter.h

#ifndef SVGEXPORTER_H

#define SVGEXPORTER_H

class SvgExporter {
public:
  SvgExporter(class Schem const &schem);
  ~SvgExporter();
  SvgExporter(SvgExporter const &) = delete;
  SvgExporter &operator=(SvgExporter const &) = delete;
public:
  bool exportSvg(class QString const &fn); // true if OK
private:
  class SvgExporterData *d;
};

#endif
