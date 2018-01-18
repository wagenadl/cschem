// PartNumbering.cpp

#include "PartNumbering.h"
#include <QStringList>
#include <QSet>
#include <QObject>

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
  QStringList bits = symbol.split(":");
  QSet<QString> yes{"resistor", "capacitor", "inductor"};
  for (QString b: bits)
    if (yes.contains(b))
      return true;
  return false;
}

bool PartNumbering::initiallyShowName(QString symbol) {
  return symbol.startsWith("part:");
}


QString PartNumbering::nameToHtml(QString name) {
  if (name.mid(1).toDouble()>0)
    return "<i>" + name.left(1) + "</i>"
      + "<sub>" + name.mid(1) + "</sub>";
  int idx = name.indexOf("_");
  if (idx>0)
    return "<i>" + name.left(idx) + "</i>"
      + "<sub>" + name.mid(idx+1) + "</sub>";
  return name;
}

QString PartNumbering::prettyValue(QString value, QString name) {
  QString pfx = name.left(1);
  QString sfx = name.mid(1);
  if (sfx.toDouble()>0) {
    if (pfx=="R" && value.endsWith("."))
      value = value.left(value.size()-1) + QObject::tr("Ω");
    else if (pfx=="C" || pfx=="L")
      value.replace("u", QObject::tr("μ"));
  }
  if (pfx=="R" || pfx=="C" || pfx=="L") {
    // insert space after number?
  }
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
 
