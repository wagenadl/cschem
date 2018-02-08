// GedaExporter.h

#ifndef GEDAEXPORTER_H

#define GEDAEXPORTER_H

class GedaExporter {
public:
  GedaExporter(class Schem const &);
  ~GedaExporter();
  GedaExporter(GedaExporter const &) = delete;
  GedaExporter &operator=(GedaExporter const &) = delete;
public:
  bool exportGeda(class QString const &fn); // true if OK
private:
  class GedaExporterData *d;
};

#endif
