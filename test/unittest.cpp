#include "../cpcb/ui/Expression.h"
#include <QDebug>

int main(int argc, char **argv) {
  Expression inch;
  Expression mm;
  inch.setMetric(false);
  mm.setMetric(true);
  for (int i=1; i<argc; i++) {
    QString arg = argv[i];
    inch.parse(arg);
    if (inch.isValid())
      qDebug() << "in inch context: " << arg << " is " << inch.value() << "inch";
    else
      qDebug() << "in inch context: " << arg << " is invalid";

    mm.parse(arg);
    if (mm.isValid())
      qDebug() << "in metric context: " << arg << " is " << mm.value() << "mm";
    else
      qDebug() << "in metric context: " << arg << " is invalid";
  }
  return 0;
}
