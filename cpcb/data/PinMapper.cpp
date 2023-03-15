// PinMapper.cpp

#include "PinMapper.h"
#include <QMap>
#include <QSet>

class PMData {
public:
  PMData(QString ref, QStringList circuitnames, QStringList pcbnames);
  bool setcirc2pcb(QString circ, QString pcb);
public:
  QString ref;
  QStringList circuitnames;
  QStringList pcbnames;
  QMap<QString, QString> circ2pcb;
  QMap<QString, QString> pcb2circ;
  QMap<QString, QString> pcb2improved;
  bool contradict;
  bool complete;
public:
  static QMap<QString, QSharedPointer<PMData>> known;
};

QMap<QString, QSharedPointer<PMData>> PMData::known;

PMData::PMData(QString ref, QStringList cc, QStringList pp):
  ref(ref) {
  // drop "nc" like names
  for (QString s: cc) {
    bool isnc = false;
    for (QString p: s.split("/"))
      if (p.toLower()=="nc" || p.toLower()=="n.c.")
	isnc = true;
    if (!isnc)
      circuitnames << s;
  }
  for (QString s: pp) {
    bool isnc = false;
    for (QString p: s.split("/"))
      if (p.toLower()=="nc" || p.toLower()=="n.c.")
	isnc = true;
    if (!isnc)
      pcbnames << s;
  }
  

  
  // Construct sets of as yes unmapped names
  QSet<QString> circav;
  QSet<QString> pcbav;
  for (QString s: circuitnames)
    circav << s;
  for (QString s: pcbnames)
    pcbav << s;

  // First, grab exact mappings.
  QList<QString> universe(circav.begin(), circav.end());
  for (QString s: universe) {
    if (pcbav.contains(s)) {
      circ2pcb[s] = s;
      pcb2circ[s] = s;
      pcb2improved[s] = s;
      circav.remove(s);
      pcbav.remove(s);
    }
  }

  // Extract numeric and nonnumeric parts of pcb pin names
  QMap<QString, QString> pcbalphapart2full;
  QMap<QString, QString> pcbnumberpart2full;
  for (QString s: pcbav) {
    QStringList ss = s.split("/");
    for (QString p: ss) {
      if (p.size()>0) {
	if (p.toInt()>0)
	  pcbnumberpart2full[p] = s;
	else
	  pcbalphapart2full[p] = s;
      }
    }
  }
  
  // Next, match things by alpha part (or any other nonnumeric part)
  for (QString s: circav) {
    QStringList ss = s.split("/");
    for (QString p: ss) {
      if (pcbalphapart2full.contains(s)) {
	QString pcb = pcbalphapart2full[s];
	if (pcbav.contains(pcb)) {
	  circ2pcb[s] = pcb;
	  pcb2circ[pcb] = s;
	  pcb2improved[s] = s;
	  circav.remove(s);
	  pcbav.remove(pcb);
	}
      }
    }
  }

  // Finally, match things by number part
  for (QString s: circav) {
    QStringList ss = s.split("/");
    for (QString p: ss) {
      if (pcbnumberpart2full.contains(s)) {
	QString pcb = pcbnumberpart2full[s];
	if (pcbav.contains(pcb)) {
	  circ2pcb[s] = pcb;
	  pcb2circ[pcb] = s;
	  pcb2improved[s] = s;
	  circav.remove(s);
	  pcbav.remove(pcb);
	}
      }
    }
  }

  complete = circav.isEmpty();

  /* Lastly, assess contradictions. A contradiction arises if any
     two circuit pins have the same number or name, or if any two
     pcbnames have the same number or name. */
  contradict = false;
  QSet<QString> circparts;
  for (QString s: circuitnames) {
    QStringList ss = s.split("/");
    for (QString p: ss) {
      if (circparts.contains(p)) {
	contradict = true;
	return;
      }
      circparts << p;
    }
  }
  QSet<QString> pcbparts;
  for (QString s: pcbnames) {
    QStringList ss = s.split("/");
    for (QString p: ss) {
      if (pcbparts.contains(p)) {
	contradict = true;
	return;
      }
      pcbparts << p;
    }
  }
}


PinMapper::PinMapper(QString ref,
		     QStringList circuitnames, QStringList pcbnames) {
  if (PMData::known.contains(ref)) {
    QSharedPointer<PMData> sp(PMData::known[ref]);
    if (sp->circuitnames==circuitnames && sp->pcbnames == pcbnames) {
      d = sp;
      return;
    }
  }
  d = QSharedPointer<PMData>(new PMData(ref, circuitnames, pcbnames));
  PMData::known[ref] = d;
}

bool PinMapper::isComplete() const {
  return d->complete;
}

bool PinMapper::isContradictory() const {
  return d->contradict;
}

QString PinMapper::pcbNameToCircuit(QString s) const {
  auto it = d->pcb2circ.find(s);
  if (it==d->pcb2circ.end())
    return "";
  else
    return *it;
}

QString PinMapper::circuitNameToPCB(QString s) const {
  auto it = d->circ2pcb.find(s);
  if (it==d->circ2pcb.end())
    return "";
  else
    return *it;
}
  
QString PinMapper::improvePCBName(QString s) const {
  auto it = d->pcb2improved.find(s);
  if (it==d->pcb2improved.end())
    return "";
  else
    return *it;
}

bool PinMapper::setCircuitNameToPCB(QString circuitname, QString pcbname) {
  return d->setcirc2pcb(circuitname, pcbname);
}

bool PMData::setcirc2pcb(QString circ, QString pcb) {
  if (!circuitnames.contains(circ))
    return false;
  if (!pcbnames.contains(pcb))
    return false;

  circ2pcb[circ] = pcb;
  pcb2circ[pcb] = circ;

  // find the best way to refer to the circuit name
  QString bestname;
  for (QString p: circ.split("/"))
    if (bestname.isEmpty() || (!p.isEmpty() && p.toInt()<=0))
      bestname = p;

  QString pcbref = pcb.split("/").first();
  pcb2improved[pcb] = pcbref + "/" + bestname;
  complete = circ2pcb.size() == circuitnames.size();
  return true;
}  
