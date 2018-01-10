// Partinfo.h

#ifndef PARTINFO_H

#define PARTINFO_H

#include <QSharedData>
#include <QXmlStreamReader>

class PartInfo {
public:
  PartInfo();
  PartInfo(PartInfo const &);
  PartInfo(QXmlStreamReader &src);
  PartInfo &operator=(PartInfo const &);
  ~PartInfo();
public:
  int id() const;
  QString notes() const;
  QString package() const;
  QString vendor() const;
  QString partno() const;
  QString mfgPart() const;
  QString manufacturer() const;
public:
  void setId(int);
  void setNotes(QString);
  void setPackage(QString);
  void setVendor(QString);
  void setPartno(QString);
  void setMfgPart(QString);
  void setManufacturer(QString);
private:
  QSharedDataPointer<class PartInfoData> d;
};

QXmlStreamWriter &operator<<(QXmlStreamWriter &, PartInfo const &);
QXmlStreamReader &operator>>(QXmlStreamReader &, PartInfo &);


#endif
