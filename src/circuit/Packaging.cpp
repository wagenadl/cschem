// Packaging.cpp

#include "Packaging.h"

#include <QDebug>

QXmlStreamReader &operator>>(QXmlStreamReader &sr, Packaging &c) {
  c = Packaging();
  while (!sr.atEnd()) {
    sr.readNext();
    if (sr.isStartElement()) {
      auto n = sr.name();
      if (n=="cschem") {
         // we'll recurse into this
      } else if (n=="packaging") {
        // we'll recurse into this
      } else if (n=="rule") {
	PkgRule rule;
	sr >> rule;
	c.rules[rule.symbol] = rule;
      } else if (n=="packages") {
        auto const &a = sr.attributes();
        QStringList names = a.value("names").toString().split(" ");
        for (QString n: names) {
          Package pkg;
          pkg.name = n;
          pkg.pcb = n.toUpper();
          c.packages[pkg.name] = pkg;
        }
        sr.skipCurrentElement();
      } else if (n=="package") {
	Package pkg;
	sr >> pkg;
	c.packages[pkg.name] = pkg;
      } else {
	qDebug() << "Unexpected element in packaging: " << sr.name();
        sr.skipCurrentElement();
      }
    } else if (sr.isEndElement()) {
      break;
    } else if (sr.isCharacters() && sr.isWhitespace()) {
    } else if (sr.isComment()) {
    } else if (sr.isStartDocument()) {
    } else {
      qDebug() << "Unexpected entity in packaging: " << sr.tokenType();
    }
  }
  // now at end of circuit element
  return sr;
}
  
QXmlStreamWriter &operator<<(QXmlStreamWriter &sr, Packaging const &c) {
  sr.writeStartElement("packaging");
  for (auto const &c: c.rules)
    sr << c;
  for (auto const &c: c.packages)
    sr << c;
  sr.writeEndElement();
  return sr;
}

Packaging &Packaging::operator+=(Packaging const &o) {
  for (QString k: o.packages.keys())
    packages[k] = o.packages[k];
  for (QString k: o.rules.keys())
    rules[k] = o.rules[k];
  return *this;
}

QString Packaging::recommendedPackage(QString symbol) const {
  QStringList pkgs = recommendedPackages(symbol);
  if (pkgs.isEmpty())
    return "";
  else return pkgs.first();
}

QStringList Packaging::recommendedPackages(QString symbol) const {
  /* Symbols may be matched loosely. That is, a symbol named
     "part:diode:zener" would match a default rule with symbol
     "diode", but "zener" would be a better match, and "diode:zener"
     even better.

     A practical algorithm is that a rule "A:B:C" can match a symbol
     "X:Y:A:B:C:Z" but not "A:B:Y:C". Score is 2^k for a match at the
     k-th level. So "resistor" matches "part:passive:resistor" with a
     score of 2^2 and "capacitor:polarized" matches
     "part:passive:capacitor:polarized" with a score of 2^2+2^3. Rule
     "diode:LED" matches "part:diode:LED" with a score of 2^1+2^2,
     outscoring rule "diode", which only scores 2^1, and rule "LED",
     which scores 2^2.

     Container symbols like "part:container:X" automatically match to
     package "X".

     Ideally, rules should be checked for matching pin counts, but
     that is complicated by containers that are represented in the
     circuit by multiple symbols (e.g., two "part:ic:virtual:opamp"
     and one "part:container:opamp2"). Perhaps this is not a real
     argument, since containers generally work without an explicit
     <rule>. Another complication is that maps are not contained in
     this file if they are simple, e.g., for the dip8 <package>. That
     means that cschem does not know how many pins a dip8 has without
     looking at the pcb files.

     Containers are treated specially: part:container:XXX automatically
     matches package XXX.
*/

  QStringList sym = symbol.split(":");

  if (sym.size()==3 && sym[1]=="container") {
    return QStringList() << sym[2];
  }

  int score = 0;
  PkgRule rule;
  for (PkgRule const &r: rules) {
    QStringList key = r.symbol.split(":");
    if (key.isEmpty())
      continue; // I don't think that can happen
    int idx = sym.indexOf(key.takeFirst());
    if (idx<0)
      continue;
    int scr = 1<<idx;
    while (!key.isEmpty()) {
      if (idx+1 < sym.size() && sym[++idx] == key.takeFirst()) {
	scr += 1<<idx;
      } else {
	scr = 0;
	break;
      }
    }
    if (scr>score) {
      score = scr;
      rule = r;
    }
  }
  return rule.packages;
}

