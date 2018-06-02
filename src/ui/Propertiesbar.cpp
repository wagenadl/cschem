// Propertiesbar.cpp

#include "Propertiesbar.h"
#include "data/Dim.h"
#include <QDoubleSpinBox>
#include <QVBoxLayout>
#include <QToolButton>
#include <QLabel>
#include <QCheckBox>
#include <QRadioButton>
#include <QToolButton>
#include <QLineEdit>

class NarrowEditor: public QLineEdit {
public:
  QSize sizeHint() const override { return minimumSizeHint(); }
};

class PBData {
public:
  Propertiesbar *parent;
  
  QWidget *xyg; // group for xy
  QWidget *xc; // container for x
  QDoubleSpinBox *x;
  QWidget *yc;
  QDoubleSpinBox *y;
  
  QWidget *dimg; // group for other dimensions
  QWidget *linewidthc;
  QDoubleSpinBox *linewidth; // for trace, hole
  QWidget *wc; 
  QDoubleSpinBox *w; // for pad
  QWidget *hc;
  QDoubleSpinBox *h; // for pad
  QWidget *idc;
  QDoubleSpinBox *id; // for hole
  QWidget *odc;
  QDoubleSpinBox *od; // for hole
  QWidget *squarec;
  QToolButton *circle; // for hole
  QToolButton *square; // for hole

  QWidget *textg;
  QDoubleSpinBox *fs; // for text
  QLineEdit *text;

  QWidget *refg;
  QLineEdit *ref;
  
  QWidget *componentg;
  QToolButton *component; // popup for replacing component

  QWidget *layerg;
  QWidget *layerc;
  QToolButton *silk;
  QToolButton *top;
  QToolButton *bottom;

  bool metric;
public:
  void switchToMetric();
  void switchToInch();
  Dim getDim(QDoubleSpinBox *);
  void setDim(QDoubleSpinBox *, Dim const &);
  void setupUI();
};

void PBData::setupUI() {
    
  auto makeGroup = [this]() {
    Q_ASSERT(parent);
    QWidget *group = new QWidget;
    auto *lay = new QVBoxLayout;
    lay->setSpacing(8);
    lay->setContentsMargins(0, 0, 0, 12);
    group->setLayout(lay);
    parent->addWidget(group);
    return group;
  };
  auto makeContainer = [](QWidget *group) {
    Q_ASSERT(group);
    Q_ASSERT(group->layout());
    QWidget *container = new QWidget;
    auto *lay = new QHBoxLayout;
    lay->setSpacing(4);
    lay->setContentsMargins(0, 0, 0, 0);
    container->setLayout(lay);
    group->layout()->addWidget(container);
    return container;
  };

  
  auto makeDimSpinner = [](QWidget *container,
			       double step=.005) {
    Q_ASSERT(container);
    Q_ASSERT(container->layout());
    QDoubleSpinBox *s = new QDoubleSpinBox;
    s->setDecimals(3);
    s->setSuffix("”");
    s->setSingleStep(step);
    container->layout()->addWidget(s);
    return s;
  };

  auto makeEdit = [](QWidget *container) {
    Q_ASSERT(container);
    Q_ASSERT(container->layout());
    auto *s = new NarrowEditor;
    container->layout()->addWidget(s);
    return s;
  };

  auto makeLabel = [](QWidget *container, QString txt) {
    Q_ASSERT(container);
    Q_ASSERT(container->layout());
    QLabel *s = new QLabel(txt);
    //    s->setToolTip(txt);
    container->layout()->addWidget(s);
    return s;
  };

  auto makeIcon = [](QWidget *container, QString icon) {
    Q_ASSERT(container);
    Q_ASSERT(container->layout());
    QLabel *s = new QLabel;
    s->setPixmap(QIcon(":icons/" + icon + ".svg").pixmap(32));
    s->setToolTip(icon);
    container->layout()->addWidget(s);
    return s;
  };

  auto makeRadio = [](QWidget *container, QString text) {
    Q_ASSERT(container);
    Q_ASSERT(container->layout());
    QRadioButton *s = new QRadioButton(text);
    container->layout()->addWidget(s);
    return s;
  };

  auto makeTextTool = [](QWidget *container, QString text) {
    Q_ASSERT(container);
    Q_ASSERT(container->layout());
    QToolButton *s = new QToolButton;
    s->setText(text);
    s->setCheckable(true);
    s->setAutoExclusive(true);
    container->layout()->addWidget(s);
    return s;
  };

  auto makeIconTool = [](QWidget *container, QString icon) {
    Q_ASSERT(container);
    Q_ASSERT(container->layout());
    QToolButton *s = new QToolButton;
    s->setIcon(QIcon(":icons/" + icon + ".svg"));
    s->setToolTip(icon);
    s->setCheckable(true);
    s->setAutoExclusive(true);
    container->layout()->addWidget(s);
    return s;
  };

  auto makeCheckBox = [](QWidget *container, QString text) {
    Q_ASSERT(container);
    Q_ASSERT(container->layout());
    QCheckBox *s = new QCheckBox(text);
    container->layout()->addWidget(s);
    return s;
  };
    
  
  xyg = makeGroup();
  xc = makeContainer(xyg);
  makeLabel(xc, "X");
  x = makeDimSpinner(xc, .050);
  yc = makeContainer(xyg);
  makeLabel(yc, "Y");
  y = makeDimSpinner(yc, .050);

  dimg = makeGroup();
  linewidthc = makeContainer(dimg);
  makeIcon(linewidthc, "Width")->setToolTip("Line width");
  linewidth = makeDimSpinner(linewidthc, .005);
  idc = makeContainer(dimg);
  makeLabel(idc, "ID")->setToolTip("Hole diameter");
  id = makeDimSpinner(idc, .005);
  odc = makeContainer(dimg);
  makeLabel(odc, "OD")->setToolTip("Pad diameter");
  od = makeDimSpinner(odc, .005);
  wc = makeContainer(dimg);
  makeLabel(wc, "W");
  w = makeDimSpinner(wc, .005);
  hc = makeContainer(dimg);
  makeLabel(hc, "H");
  h = makeDimSpinner(hc, .005);
  squarec = makeContainer(dimg);
  makeLabel(squarec, "Shape");
  circle = makeTextTool(squarec, "○");
  circle->setToolTip("Round");
  circle->setChecked(true);
  square = makeTextTool(squarec, "□");
  square->setToolTip("Square");

  layerg = makeGroup();
  layerc = makeContainer(layerg);
  makeLabel(layerc, "Layer");
  silk = makeIconTool(layerc, "Silk");
  top = makeIconTool(layerc, "Top");
  bottom = makeIconTool(layerc, "Bottom");

  textg = makeGroup();
  auto *c1 = makeContainer(textg);
  makeLabel(c1, "Aa")->setToolTip("Text");
  auto *c2 = makeContainer(textg);
  text = makeEdit(c2);
  fs = makeDimSpinner(c1);
  fs->setToolTip("Font size");
}  

Propertiesbar::Propertiesbar(QWidget *parent): QToolBar(parent) {
  d = new PBData;
  d->parent = this;
  d->metric = false;
  d->setupUI();
}
