// PartNumbering.cpp

#include "PartNumbering.h"

QString PartNumbering::abbreviation(QString symbol) {
  QStringList bits = symbol.split(":");
  QSet<QString> set = QSet<QString>::fromList(bits);
  if (set.contains("port"))
    return "J";
  if (set.contains("diode"))
    return "D";
  else if (set.contains("resistor"))
    return "R";
  else if (set.contains("capacitor"))
    return "C";
  else if (set.contains("opamp"))
    return "A";
  else if (set.contains("transistor"))
    return "Q";
  else if (set.contains("logic"))
    return "U";
  else if (set.contains("inductor"))
    return "L";
  else if (set.contains("battery"))
    return "B";
  else
    return "U";
}
