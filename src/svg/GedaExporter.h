// GedaExporter.h

#ifndef GEDAEXPORTER_H

#define GEDAEXPORTER_H

class GedaExporter {
public:
  GedaExporter(class Circuit const &circ, class SymbolLibrary const *lib);
  ~GedaExporter();
  GedaExporter(GedaExporter const &) = delete;
  GedaExporter &operator=(GedaExporter const &) = delete;
public:
  bool exportGeda(class QString const &fn); // true if OK
private:
  class GedaExporterData *d;
};

#endif
