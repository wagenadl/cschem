// Connection.h

#ifndef CONNECTION_H

#define CONNECTION_H

#include <QSharedData>
#include <QXmlStreamReader>

class Connection {
public:
  Connection();
  Connection(Connection const &);
  Connection(QXmlStreamReader &src);
  Connection &operator=(Connection const &);
  ~Connection();
public:
  int id() const;
  int fromId() const;
  int toId() const;
  QString fromPin() const;
  QString toPin() const;
  QList<QPoint> const &via() const;
public:
  void setId(int);
  void setFromId(int);
  void setToId(int);
  void setFromPin(QString);
  void setToPin(QString);
  void setFrom(int id, QString pin);
  void setTo(int id, QString pin);
  QList<QPoint> &via();
  void setVia(QList<QPoint> const &);
  void translate(QPoint delta);
private:
  QSharedDataPointer<class ConnectionData> d;
};

QXmlStreamWriter &operator<<(QXmlStreamWriter &, Connection const &);
QXmlStreamReader &operator>>(QXmlStreamReader &, Connection &);


#endif
