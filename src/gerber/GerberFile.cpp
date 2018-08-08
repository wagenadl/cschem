// GerberFile.cpp

#include "GerberFile.h"

using namespace Gerber;

GerberFile::GerberFile(QDir dir, Gerber::Layer layer):
  f(dir.absoluteFilePath(dir.name() + "-" + layerInfix(layer) + ".gbr")) {
  if (!f.open(QFile::ReadOnly))
    return;
  setDevice(&f);
  *this << "G04 " << dir.absolutePath() << " - " << layerInfix(layer) << " *\n";
  *this << "%TF.GenerationSoftware,cpcb,DanielWagenaar*%\n";
  *this << "%FSLAX35Y35*%\n";
  *this << "%MOMM*%\n";
}

GerberFile::~GerberFile() {
  // we do not write the M02 here, so that write errors are obvious
}

void GerberFile::writeBoilerplate(QString uuid, Polarity polarity) {
  if (polarity==Polarity::Negative) 
    *this << "%TF.FilePolarity,Negative*%\n";
  else
    *this << "%TF.FilePolarity,Positive*%\n";
  *this << "%TF.Part,Single*%\n";
  *this << "%TF.SameCoordinates," << uuid << "*%\n";
}
    

GerberFile::~GerberFile() {
}

bool GerberFile::isValid() const {
  return device() != 0;
}
