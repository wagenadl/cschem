// PartNumbering.h

#ifndef PARTNUMBERING_H

#define PARTNUMBERING_H

#include <QString>

class PartNumbering {
public:
  static QString abbreviation(QString symbol);
  static QString nameToHtml(QString name);
  static QString prettyValue(QString value, QString name="");
  static bool initiallyShowValue(QString symbol);
  static bool initiallyShowName(QString symbol);
  static QString prefix(QString name); // "R" part of "R3"
  static int number(QString name); // 3 part of "A3.2"; 0 if none
  static QString cnumber(QString name); // "3" part of "A3.2"; empty if none
  static int subNumber(QString name); // 2 part of "A3.2"; 0 if none
  static QString cname(QString name); // "A3" part of "A3.2"
  static QString csuffix(QString name); // ".2" part of "A3.2"; empty if none
  static bool isNameWellFormed(QString name);
  // a well formed name starts with one or more letters followed
  // optionally by one or more digits followed optionnaly by "." and
  // more digits.
  static bool lessThan(QString a, QString b);
  static QString compactRefs(QSet<QString> refs);
  static QString compactRefs(QStringList refs);
  static QList<QStringList> compressPartList(QList<QStringList> symbols);
  static QStringList unpackRefs(QString crefs);
};

#endif
