// PartsListEditor.cpp

#include "PartsListEditor.h"

#include <QGridLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QComboBox>
#include <QToolButton>
#include <QDialogButtonBox>

class PLEData {
public:
  PLEData(PartsListEditor *ple): ple(ple) {
    circuit = 0;
    widget = new QWidget;
    grid = new QGridLayout;
    widget->setLayout(grid);
    vlay = new QVBoxLayout;
    vlay->addWidget(widget);
    bbb = new QDialogButtonBox;
    bb_cancel = bbb->addButton(QDialogButtonBox::Cancel);
    bb_apply = bbb->addButton(QDialogButtonBox::Apply);
    bb_ok = bbb->addButton(QDialogButtonBox::Ok);
    vlay->addStretch(1);
    vlay->addWidget(bbb);
    ple->setLayout(vlay);

    rebuild();
  }
  void rebuild();
public:
  PartsListEditor *ple;
  Circuit const *circuit;
  Group root;
  QWidget *widget;
  QGridLayout *grid;
  QVBoxLayout *vlay;
  QDialogButtonBox *bbb;
  QPushButton *bb_cancel;
  QPushButton *bb_ok;
  QPushButton *bb_apply;
  QMap<QString, QLabel *> reflabels;
  QMap<QString, QLabel *> pvlabels;
  QMap<QString, QLabel *> circuitnotelabels;
  QMap<QString, QComboBox *> packagecombos;
  QMap<QString, QLineEdit *> noteeditors;
  QMap<QString, QToolButton *> notecreators;
};

void PartsListEditor::rebuild() {
  for (auto *w: reflabels)
    delete w;
  for (auto *w: pvlabels)
    delete w;
  for (auto *w: circuitnotelabels)
    delete w;
  for (auto *w: packagecombos)
    delete w;
  for (auto *w: notecreators)
    delete w;
  for (auto *w: noteeditors)
    delete w;
  reflabels.clear();
  pvlabels.clear();
  circuitnotelabels.clear();
  packagecombos.clear();
  noteeditors.clear();
  notecreators.clear();

  if (!circuit)
    return;

  QMap<QString, Group const *> components;
  for (int id: group.keys()) {
    Object const &obj(group.object(id));
    if (obj.isGroup()) {
      Group const &grp(obj.asGroup());
      components[grp.ref] = &grp;
    }
  }
  
  int row = -1;
  for (auto const &elt: circuit->elements) {
    QString ref = elt.name;
    QString pv = elt.value;
    QString notes = elt.cnotes;
    if (ref.isEmpty())
      continue;
    grid->addWidget(reflabels[ref] = new QLabel(ref), ++row, 0);
    grid->addWidget(pvlabels[ref] = new QLabel(pv), row, 1);
    grid->addWidget(packagecombos[ref] = new QComboBox(), row, 2);
    grid->addWidget(notecreators[ref] = new QToolButton("+"), row, 3);
    if (!cnotes.isEmpty())
      grid->addWidget(circuitnotelabels[ref] = new QLabel(cnotes),
		      ++row, 0, 1, 4);
    grid->addWidget(noteeditors[ref] = new QLineEdit(), ++row, 0, 1, 4);
    if (components.contains(ref)) {
      Group const &g = *components[ref];
      if (!g.pkg.isEmpty())
	packagecombos[ref].addItem(g.pkg);
      // should also access "known packages" db
      noteeditors[ref]->setText(g.notes);
    }
  }    

  for (auto *w: reflabels)
    w->show;
  for (auto *w: pvlabels)
    w->show();
  for (auto *w: circuitnotelabels)
    w->show();
  for (auto *w: packagecombos)
    w->show();
  for (QString const &ref: notecreators.keys())
    notecreators[ref]->setVisible(components.contains(ref)
				  && components[ref]->notes.isEmpty());

  for (QString const &ref: noteeditors.keys())
    noteeditors[ref]->setVisible(components.contains(ref)
				 && !components[ref]->notes.isEmpty());
}

PartsListEditor::PartsListEditor(QWidget *parent):
  QDialog(parent),
  d(new PLEData(this)) {
}

PartsListEditor::~PartsListEditor() {
  delete d;
}

void PartsListEditor::setRoot(Group const &root) {
  d->root = root;
  d->rebuild();
}

void PartsListEditor::setCircuit(Circuit const &circuit) {
  d->circuit = &circuit;
  d->rebuild();
}

