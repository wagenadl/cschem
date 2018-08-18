// Nodename.h

#ifndef NODENAME_H

#define NODENAME_H

#include <QString>
#include <QSet>

enum class MatchQuality {
  None = 0,
  Number = 1,
  Name = 2,
  Perfect = 3,
};

//inline bool operator<(MatchQuality const &a, MatchQuality const &b) {
//  return int(a) < int(b);
//}

class Nodename {
public:
  Nodename(QString component, QString pin);
  QString component() const { return comp_; }
  QString pin() const { return pin_; }
  QString toString() const;
  QString humanName() const;  
  bool isValid() const;
  bool operator==(Nodename const &o) const;
  bool matches(Nodename const &o) const;
  MatchQuality matchQuality(Nodename const &o) const;
private:
  QString comp_;
  QString pin_;
};

inline uint qHash(Nodename const &nn) {
  return qHash(QPair<QString, QString>(nn.component(), nn.pin()));
}

#endif
