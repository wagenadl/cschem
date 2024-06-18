// Apertures.cpp

#include "Apertures.h"
#include "Gerber.h"

namespace Gerber {
  QString Apertures::funcName(Apertures::Func func) {
    switch (func) {
    case Apertures::Func::Material: return "Material";
    case Apertures::Func::Conductor: return "Conductor";
    case Apertures::Func::NonConductor: return "NonConductor";
    case Apertures::Func::Profile: return "Profile";
    case Apertures::Func::ComponentDrill: return "ComponentDrill";
    case Apertures::Func::ComponentPad: return "ComponentPad";
    case Apertures::Func::SMDPad: return "SMDPad,CuDef";
    case Apertures::Func::AntiPad: return "AntiPad";
    case Apertures::Func::Invalid: break;
    }
    qDebug() << "Apertures::funcName(Invalid)";
    return "XXX";
  }

  Apertures::Apertures(Apertures::Func func, int apidx, bool useattr):
    func_(func), apidx(apidx), useattr(useattr) {
  }

  Apertures::Func Apertures::func() const {
    return func_;
  }
  
  QTextStream &operator<<(QTextStream &ts, Apertures const &ap) {
    ap.write(ts);
    return ts;
  }
  
  void Apertures::write(QTextStream &output) const {
    if (apCirc.isEmpty() && apRect.isEmpty()
	&& apHole.isEmpty() && apSqHole.isEmpty())
      return;
    if (useattr)
      output << "%TA.AperFunction," << funcName(func()) << "*%\n";    
    for (Circ const &k: apCirc.keys())
      output << "%ADD" << apCirc[k]
	     << "C," << real(k.diam) << "*%\n";
    for (Rect const &k: apRect.keys())
      output << "%ADD" << apRect[k]
	     << "R," << real(k.w) << "X" << real(k.h) << "*%\n";
    for (Hole const &k: apHole.keys())
      output << "%ADD" << apHole[k]
	     << "C," << real(k.od) << "," << real(k.id) << "*%\n";
    for (SqHole const &k: apSqHole.keys())
      output << "%ADD" << apSqHole[k]
	     << "R," << real(k.wh) << "," << real(k.wh)
	     << "," << real(k.id) << "*%\n";
  }

  QString Apertures::select(Circ k) const {
    Q_ASSERT(apCirc.contains(k));
    return "D" + QString::number(apCirc[k]) + "*\n";
  }

  QString Apertures::select(Rect k) const {
    Q_ASSERT(apRect.contains(k));
    return "D" + QString::number(apRect[k]) + "*\n";
  }

  QString Apertures::select(Hole k) const {
    Q_ASSERT(apHole.contains(k));
    return "D" + QString::number(apHole[k]) + "*\n";
  }

  QString Apertures::select(SqHole k) const {
    Q_ASSERT(apSqHole.contains(k));
    return "D" + QString::number(apSqHole[k]) + "%\n";
  }

  void Apertures::ensure(Circ k) {
    if (apCirc.contains(k))
      return;
    apCirc[k] = apidx++;
  }

  void Apertures::ensure(Rect k) {
    if (apRect.contains(k))
      return;
    apRect[k] = apidx++;
  }

  void Apertures::ensure(Hole k) {
    if (apHole.contains(k))
      return;
    apHole[k] = apidx++;
  }

  void Apertures::ensure(SqHole k) {
    if (apSqHole.contains(k))
      return;
    apSqHole[k] = apidx++;
  }

  int Apertures::firstIndex() const {
    int idx = apidx;
    for (int id: apCirc)
      if (id<idx)
	idx = id;
    for (int id: apRect)
      if (id<idx)
	idx = id;
    for (int id: apHole)
      if (id<idx)
	idx = id;
    for (int id: apSqHole)
      if (id<idx)
	idx = id;
    return idx;
  }
};
