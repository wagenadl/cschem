// PinMapDialog.cpp

#include "PinMapDialog.h"
#include "VerticalLabel.h"
#include <QVBoxLayout>
#include <QGridLayout>
#include <QDialogButtonBox>
#include <QRadioButton>
#include <QDebug>

class PMDData {
public:
  PMDData(PinMapDialog *pmd): pmd(pmd) {
  }
  void setupGUI();
  bool isValid() const;
  void rebuildGUI();
  void refillTable();
  void cleanupMap();
public:
  PinMapDialog *pmd;
  QVBoxLayout *mainlayout;
  QGridLayout *outertable;
  QGridLayout *innertable;
  QList<QLabel *> circnamelbl;
  QList<QLabel *> pcbnamelbl;
  QMap<int, QMap<int, QRadioButton *>> table;
  QLabel *mainlbl;
  QWidget *tablewidget;
  QDialogButtonBox *buttons;
public:
  QStringList circuitnames;
  QStringList pcbnames;
  QMap<QString, QString> pcb2circ;
  QString refname;
};

void PMDData::setupGUI() {
  mainlayout = new QVBoxLayout();
  mainlbl = new QLabel("Pin map");
  mainlbl->setAlignment(Qt::AlignCenter);
  mainlayout->addWidget(mainlbl);

  outertable = new QGridLayout();
  outertable->addWidget(new QLabel("PCB names"), 0, 1);
  outertable->addWidget(new VerticalLabel("Circuit names"), 1, 0);
  tablewidget = new QWidget;
  innertable = new QGridLayout();
  tablewidget->setLayout(innertable);
  outertable->addWidget(tablewidget, 1, 1);

  buttons = new QDialogButtonBox(QDialogButtonBox::Ok
				 | QDialogButtonBox::Cancel);
  QObject::connect(buttons, &QDialogButtonBox::accepted,
	  [this]() { pmd->accept(); });
  QObject::connect(buttons, &QDialogButtonBox::rejected,
	  [this]() { pmd->reject(); });
  mainlayout->addWidget(buttons);
  
  pmd->setLayout(mainlayout);
}

void PMDData::rebuildGUI() {
  for (auto *w: circnamelbl)
    delete w;
  for (auto *w: pcbnamelbl)
    delete w;
  for (auto const &row: table)
    for (auto *w: row)
      delete w;

  circnamelbl.clear();
  pcbnamelbl.clear();
  table.clear();

  for (int k=0; k<circuitnames.size(); k++) {
    auto *w = new QLabel(circuitnames[k]);
    circnamelbl << w;
    innertable->addWidget(w, k+1, 0);
  }
  for (int n=0; n<pcbnames.size(); n++) {
    auto *w = new QLabel(pcbnames[n]);
    pcbnamelbl << w;
    innertable->addWidget(w, 0, n+1);
  }
  for (int k=0; k<circuitnames.size(); k++) {
    for (int n=0; n<pcbnames.size(); n++) {
      auto *w = new QRadioButton();
      w->setAutoExclusive(false);
      table[n][k] = w;
      QObject::connect(w, &QRadioButton::clicked,
		       [this, n, k]() {
			 for (int k1=0; k1<table[n].size(); k1++)
			   table[n][k1]->setChecked(k==k1);
			 pcb2circ[pcbnames[n]] = circuitnames[k];
		       });
      innertable->addWidget(w, k+1, n+1);
    }
  }
  refillTable();
}

void PMDData::refillTable() {
  for (int n=0; n<pcbnames.size(); n++) {
    QString pcb = pcbnames[n];
    int k1 = -1;
    if (pcb2circ.contains(pcb)) {
      QString circ = pcb2circ[pcb];
      k1 = circuitnames.indexOf(circ);
    }
    for (int k=0; k<circuitnames.size(); k++)
      table[n][k]->setChecked(k==k1);
  }
}

void PMDData::cleanupMap() {
  for (QString pcb: pcb2circ.keys())
    if (!pcbnames.contains(pcb)
	|| !circuitnames.contains(pcb2circ[pcb]))
      pcb2circ.remove(pcb);
}

PinMapDialog::PinMapDialog(QWidget *parent):
  QDialog(parent), d(new PMDData(this)) {
  d->setupGUI();
}

PinMapDialog::~PinMapDialog() {
  delete d;
}

void PinMapDialog::setReference(QString ref) {
  d->refname = ref;
  if (ref.isEmpty())
    d->mainlbl->setText("Pin map");
  else
    d->mainlbl->setText("Pin map for " + ref);
}

void PinMapDialog::setCircuitNames(QStringList const &ss) {
  d->circuitnames = ss;
  d->cleanupMap();
  d->rebuildGUI();
}

void PinMapDialog::setPCBNames(QStringList const &ss) {
  d->pcbnames.clear();
  for (QString const &s: ss)
    d->pcbnames << stripPCBName(s);
  d->cleanupMap();
  d->rebuildGUI();
}

void PinMapDialog::setMap(QMap<QString, QString> const &m) {
  for (QString const &s: m.keys())
    d->pcb2circ[stripPCBName(s)] = m[s];
  d->cleanupMap();
  d->refillTable();
}

void PinMapDialog::setMapping(QString pcb, QString circ) {
  d->pcb2circ[stripPCBName(pcb)] = circ;
  d->cleanupMap();
  d->refillTable();
}

void PinMapDialog::autoMap() {
  qDebug() << "automap nyi";
}

QMap<QString, QString> PinMapDialog::map() const {
  return d->pcb2circ;
}

QString PinMapDialog::map(QString s) const {
  s = stripPCBName(s);
  auto it = d->pcb2circ.find(s);
  if (it==d->pcb2circ.end())
    return "";
  else
    return *it;
}

QString PinMapDialog::stripPCBName(QString s) {
  int idx = s.indexOf("/");
  if (idx>=0)
    return s.left(idx);
  else
    return s;
}
