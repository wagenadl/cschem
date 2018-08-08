// Aperture.cpp

#include "Aperture.h"
#include "Gerber.h"

namespace Gerber {
  Aperture::Aperture() {
    apidx = 200;
  }

  void Aperture::write(QTextStream &output) {
    for (Circ const &k: apCirc.keys())
      output << "%ADD" << apCirc[k]
	     << "C," << real(k.diam) << "*%\n";
    for (Rect const &k: apRect.keys())
      output << "%ADD" << apRect[k]
	     << "R," << real(k.w) << "," << real(k.h) << "*%\n";
    for (Hole const &k: apHole.keys())
      output << "%ADD" << apHole[k]
	     << "C," << real(k.od) << "," << real(k.id) << "*%\n";
    for (SqHole const &k: apSqHole.keys())
      output << "%ADD" << apSqHole[k]
	     << "R," << real(k.wh) << "," << real(k.wh)
	     << "," << real(k.id) << "*%\n";
  }

  QString Aperture::select(Circ k) const {
    Q_ASSERT(apCirc.contains(k));
    return "%D" + QString::number(apCirc[k]) + "*%\n";
  }

  QString Aperture::select(Rect k) const {
    Q_ASSERT(apRect.contains(k));
    return "%D" + QString::number(apRect[k]) + "*%\n";
  }

  QString Aperture::select(Hole k) const {
    Q_ASSERT(apHole.contains(k));
    return "%D" + QString::number(apHole[k]) + "*%\n";
  }

  QString Aperture::select(SqHole k) const {
    Q_ASSERT(apSqHole.contains(k));
    return "%D" + QString::number(apSqHole[k]) + "*%\n";
  }

  void Aperture::ensure(Circ k) {
    if (apCirc.contains(k))
      return;
    apCirc[k] = ++apidx;
  }

  void Aperture::ensure(Rect k) {
    if (apRect.contains(k))
      return;
    apRect[k] = ++apidx;
  }

  void Aperture::ensure(Hole k) {
    if (apHole.contains(k))
      return;
    apHole[k] = ++apidx;
  }

  void Aperture::ensure(SqHole k) {
    if (apSqHole.contains(k))
      return;
    apSqHole[k] = ++apidx;
  }

};
