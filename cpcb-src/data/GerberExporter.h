// GerberExporter.h

#ifndef GERBEREXPORTER_H

#define GERBEREXPORTER_H

class GerberExporter {
public:
  GerberExporter(class QTextStream &output);
  ~GerberExporter();
  void writeComment(QString text);
  void writeLayout(class Layout const &layout);
private:
  class GEData *d;
};

#endif
