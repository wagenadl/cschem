// GerberFile.h

#ifndef GERBERFILE_H

#define GERBERFILE_H

#include "Gerber.h"
#include <QTextStream>

class GerberFile: public QTextStream {
public:
  GerberFile(QDir dir, Gerber::Layer layer);
  ~GerberFile();
  bool isValid() const;
  void writeBoilerplate(QString uuid,
			Gerber::Polarity polarity=Gerber::Polarity::Positive);
private:
  QFile f;
};

#endif
