// CSV.cpp

#include "CSV.h"

namespace CSV {
  QString quote(QString s) {
    s.replace("\"", "\"\"");
    return "\"" + s + "\"";
 }

  QString encode(QList<QStringList> table) {
    QString res = "";
    for (QStringList line: table) {
      QString sep = "";
      for (QString cell: line) {
        res += sep;
        res += quote(cell);
        sep = ", ";
      }
      res += "\n";
    }
    return res.replace("µ", "u");
  }

  QList<QStringList> decode(QString csv) {
    QList<QStringList> table;
    QStringList row;
    QString cell;
    bool instring = false;
    int len = csv.size();
    const QChar newline = QChar('\n');
    const QChar cr = QChar('\r');
    const QChar dquote = QChar('"');
    const QChar comma = QChar(',');
    for (int pos=0; pos<len; pos++) {
      QChar c=csv[pos];
      if (instring) {
        if (c==dquote) {
          if (pos<len-1 && csv[pos+1]==dquote) {
            cell += dquote;
            pos++;
          } else {
            instring = false;
          }
        } else {
          cell += c;
        }
      } else {
        if (c==dquote) {
          instring = true;
        } else if (c==comma) {
          row << cell;
          cell = "";
        } else if (c==cr) {
          //
        } else if (c==newline) {
          row << cell;
          table << row;
          cell = "";
          row = QStringList();
        } else {
          cell += c;
        }
      }
    }
    if (row.size() || cell.size()) {
      row << cell;
      table << row;
    }
    return table;
  }
};

    
