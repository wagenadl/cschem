// PartsList.h

#ifndef PARTSLIST_H

#define PARTSLIST_H

#include <QSharedData>
#include <QXmlStreamReader>

class PartsList {
public:
  PartsList();
  PartsList(PartsList const &);
  PartsList &operator=(PartsList const &);
  ~PartsList();
public:
  void insert(class Part const &); // or replace
  void remove(int id);
  Part const &findPart(QString name); // name can be "Q1" or "Q1.1" or "A1",
  // or perhaps even "[A1,A2]"
  int maxId() const;
  int renumber(int start=1);
  QMap<int, class Part> const &parts() const;
  Part const &part(int) const;
  bool isEmpty() const;
  bool isValid() const;
private:
  QSharedDataPointer<class PartsListData> d;
  friend QXmlStreamWriter &operator<<(QXmlStreamWriter &, PartsList const &);
  friend QXmlStreamReader &operator>>(QXmlStreamReader &, PartsList &);
};

QXmlStreamWriter &operator<<(QXmlStreamWriter &, PartsList const &);
QXmlStreamReader &operator>>(QXmlStreamReader &, PartsList &);

#endif
