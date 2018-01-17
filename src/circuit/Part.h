// Part.h

#ifndef PART_H

#define PART_H

#include <QSharedData>
#include <QXmlStreamReader>

class Part {
public:
  Part();
  Part(Part const &);
  Part &operator=(Part const &);
  ~Part();
  QString report() const;
public:
  bool isValid() const;
  QString name() const; // "Q1", "[A1,A2]"
  QString value() const; // e.g., "10 pF" or "INA111"
  QString vendor() const;
  QString partno() const;
  QString package() const;
  QString notes() const;
  int id() const;
public:
  void setName(QString);
  void setValue(QString);
  void setVendor(QString);
  void setPartno(QString);
  void setPackage(QString);
  void setNotes(QString);
  void setId(int); // should only be used for well-controlled renumber op
protected:
  friend QXmlStreamWriter &operator<<(QXmlStreamWriter &, Part const &);
  friend QXmlStreamReader &operator>>(QXmlStreamReader &, Part &);
  virtual void writeAttributes(QXmlStreamWriter &) const;
  virtual void readAttributes(QXmlStreamReader &);
private:
  QSharedDataPointer<class PartData> d;
};

QXmlStreamWriter &operator<<(QXmlStreamWriter &, Part const &);
QXmlStreamReader &operator>>(QXmlStreamReader &, Part &);


#endif
