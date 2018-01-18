// PartNumbering.h

#ifndef PARTNUMBERING_H

#define PARTNUMBERING_H

#include <QString>

class PartNumbering {
public:
  static QString abbreviation(QString symbol);
  static QString nameToHtml(QString name);
  static QString prettyValue(QString value, QString name="");
  static QString shortValue(QString value, QString name="");
  static QString htmlToSvg(QString html);
  static bool initiallyShowValue(QString symbol);
  static bool initiallyShowName(QString symbol);
};

#endif
