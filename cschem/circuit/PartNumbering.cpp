// PartNumbering.cpp

#include "PartNumbering.h"
#include <QStringList>
#include <QSet>
#include <QObject>
#include <QRegularExpression>
#include <QDebug>

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
