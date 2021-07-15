// CSV.h

#ifndef CSV_H

#define CSV_H

#include <QStringList>

namespace CSV {
  QString quote(QString s);
  QString encode(QList<QStringList> table);
  QList<QStringList> decode(QString csv);
};

#endif
