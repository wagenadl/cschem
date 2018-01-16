// Exporter.h

#ifndef EXPORTER_H

#define EXPORTER_H

class Exporter {
public:
  Exporter(class Circuit const &circ, class SymbolLibrary const *lib);
  ~Exporter();
  Exporter(Exporter const &) = delete;
  Exporter &operator=(Exporter const &) = delete;
public:
  bool exportSvg(class QString const &fn); // true if OK
private:
  class ExporterData *d;
};

#endif
