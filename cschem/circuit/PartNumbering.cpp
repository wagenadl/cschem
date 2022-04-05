// PartNumbering.cpp

#include "PartNumbering.h"
#include <QStringList>
#include <QSet>
#include <QObject>
#include <QRegularExpression>
#include <QDebug>
#include <algorithm>


QString PartNumbering::abbreviation(QString symbol) {
  static QStringList map{
    "connector", "J",
    "diode", "D",
    "resistor", "R",
    "capacitor", "C",
    "opamp", "A",
    "transistor", "Q",
    "logic", "U",
    "inductor", "L",
    "battery", "B"
  };
  
  QStringList bits = symbol.split(":");
  QSet<QString> set = QSet<QString>::fromList(bits);
  for (int k=0; k<map.size(); k+=2)
    if (set.contains(map[k]))
      return map[k+1];
  return "U";
}

bool PartNumbering::initiallyShowValue(QString symbol) {
  static QSet<QString> yes{"resistor", "capacitor", "inductor"};
  QStringList bits = symbol.split(":");
  for (QString b: bits)
    if (yes.contains(b))
      return true;
  return false;
}

bool PartNumbering::initiallyShowName(QString symbol) {
  return symbol.startsWith("part:");
}


QString PartNumbering::nameToHtml(QString name) {
  if (name.left(1)=="V" || name.left(1)=="I")
    return "<i>V</i><sub>" + name.mid(1) + "</sub>";
  else if (isNameWellFormed(name))
    return "<i>" + prefix(name) + "</i>"
      + "<sub>" + cnumber(name) + csuffix(name) + "</sub>";
  else
    return name;
}

QString PartNumbering::prettyValue(QString value, QString name) {
  value.replace("Ohm", "Ω");
  value.replace("uF", "μF");
  value.replace("uH", "μH");
  if (isNameWellFormed(name) && prefix(name)=="R")
    if (value.endsWith("."))
      value = value.left(value.size()-1) + "Ω";
  if (value.startsWith('"'))
    value = QString("“") + value.mid(1);
  if (value.endsWith('"'))
    value = value.left(value.size()-1) + QString("”");
  return value;
}

QString PartNumbering::shortValue(QString value, QString) {
  // remove space after number
  // remove Ohm, F, ?
  return value;
}

QString PartNumbering::htmlToSvg(QString html) {
  return html;
}
 
static QRegularExpression wfn("^([A-Za-z]+)((\\d+)(.(\\d+))?)?$");
// e.g., "A1.2"

bool PartNumbering::isNameWellFormed(QString name) {
  return wfn.match(name).hasMatch();
}

QString PartNumbering::prefix(QString name) {
  return wfn.match(name).captured(1); // the "A" part
}

QString PartNumbering::cnumber(QString name) {
  return wfn.match(name).captured(3); // the "1" part, as string
}

int PartNumbering::number(QString name) {
  return wfn.match(name).captured(3).toInt(); // the "1" part
}

int PartNumbering::subNumber(QString name) {
  return wfn.match(name).captured(5).toInt(); // the "2" part
}

QString PartNumbering::cname(QString name) {
  auto m(wfn.match(name));
  return m.captured(1) + m.captured(3); // // the "A1" part
}

QString PartNumbering::csuffix(QString name) { // the ".2" part
  return wfn.match(name).captured(4);
}

bool PartNumbering::lessThan(QString a, QString b) {
  auto unpack = [](QString s) {
                  QList<QVariant> l;
                  bool isnum = false;
                  bool skip = false;
                  int num = 0;
                  for (QChar c: s) {
                    if (skip) {
                      if (c=='>')
                        skip = false;
                    } else if (c=='<') {
                      skip = true;
                    } else if (c.isDigit()) {
                      isnum = true;
                      num = 10*num + (c.unicode()-'0');
                    } else {
                      if (isnum)
                        l << num;
                      isnum = false;
                      num = 0;
                      l << c;
                    }
                  }
                  if (isnum)
                    l << num;
                  return l; };
  return unpack(a) < unpack(b);
}

  
QString PartNumbering::compactRefs(QSet<QString> refs) {
  QList<QString> reflist = refs.toList();
  std::sort(reflist.begin(), reflist.end(), PartNumbering::lessThan);
  /* I want the list [C1, C3, C5, C6, C7, C10, C11, C15] to be represented
     as "C1,3,5–7,10,11,15."
     I could also want "C1, C3, C5–7, C10–11, C15." I think that's better.
     Or "C1, 3, 5–7, 10–11, 15," but I prefer to keep the prefixes
     I do not anticipate seeing csuffixes here, but if I do, I suppose that
     [A1, A2, A3.1, A3.2, A3.3, A3.7, A4] should be represented as
     "A1, A2, A3.1–3.3, A3.7, A4."
     That means that for present purposes, a part number like "A3.2" 
     effectively has "A3." as a prefix and 2 as a number
  */
  QString prefix = "";
  int first = -1;
  int last = -1;
  QStringList bits;
  auto stringify = [](QString prefix, int first, int last) {
    if (last>first)
      return QString("%1%2–%3").arg(prefix).arg(first).arg(last);
    else
      return QString("%1%2").arg(prefix).arg(first);
  };

  for (QString ref: reflist) {
    QString sfx = csuffix(ref);
    QString pfx;
    if (sfx=="") 
      sfx = cnumber(ref);
    pfx = ref.left(ref.size() - sfx.size());
    if (pfx != prefix) {
      if (prefix!="") {
	// add previous build
	bits.append(stringify(prefix, first, last));
	prefix = "";
      }
      first = last = sfx.toInt();
      if (first) 
	prefix = pfx; // keep for later
      else
	bits.append(pfx); // right away
    } else {
      int num = sfx.toInt();
      if (num==last+1) {
	last++;
      } else {
	bits.append(stringify(prefix, first, last));
	first = last = num;
      }
    }
  }
  if (prefix!="") 
    // add final build
    bits.append(stringify(prefix, first, last));

  return bits.join(", ");
}

struct KeyType {
  KeyType(QString pfx="", QString val="", QString notes=""):
    pfx(pfx), val(val), notes(notes) { }
  QString pfx;
  QString val;
  QString notes;
};

bool operator<(KeyType const &a, KeyType const &b) {
  if (a.pfx != b.pfx)
    return a.pfx < b.pfx;
  if (a.val != b.val)
    return a.val < b.val;
  return a.notes < b.notes;
}

QList<QStringList> PartNumbering::compressPartList(QList<QStringList> symbols) {
  QMap<KeyType, QSet<QString>> map;
  // key is (VALUE, NOTES), value is set of refs
  for (QStringList const &line: symbols) {
    if (line.size()<2)
      continue;
    QString ref = line[0];
    QString val = line[1];
    QString notes = line.size()>=2 ? line[2] : "";
    KeyType key(prefix(ref), val, notes);
    if (!map.contains(key))
      map[key] = QSet<QString>();
    map[key].insert(ref);
  }

  QMap<QString, QString> values;
  QMap<QString, QString> notes;
  QStringList keys;
  for (auto it=map.begin(); it!=map.end(); ++it) {
    QString key = PartNumbering::compactRefs(it.value());
    keys << key;
    values[key] = it.key().val;
    notes[key] = it.key().notes;
  }
  std::sort(keys.begin(), keys.end(), PartNumbering::lessThan);
  QList<QStringList> result;
  for (QString key: keys) 
    result << QStringList({key, values[key], notes[key]});
  return result;
}
