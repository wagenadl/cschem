// PasteMaskWriter.h

#ifndef PASTEMASKWRITER_H

#define PASTEMASKWRITER_H

#include "data/Dim.h"
#include <QString>

class PasteMaskWriter {
public:
  PasteMaskWriter();
  ~PasteMaskWriter();
  void setShrinkage(Dim);
  bool write(class Layout const &layout, QString filename);
private:
  PasteMaskWriter(PasteMaskWriter const &) = delete;
  PasteMaskWriter &operator=(PasteMaskWriter const &) = delete;
  class PMWData *d;
};

#endif
