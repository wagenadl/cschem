// Propertiesbar.cpp

#include "Propertiesbar.h"
#include "data/Dim.h"
#include "ui/Editor.h"
#include "DimSpinner.h"
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
  Editor *editor;
  Mode mode;
  
  QWidget *xyg; // group for xy
  QAction *xya; // group for xy
  QWidget *xc; // container for x
  DimSpinner *x;
  QWidget *yc;
  DimSpinner *y;
  
  QWidget *dimg; // group for other dimensions
  QAction *dima; // group for other dimensions
  QWidget *linewidthc;
  DimSpinner *linewidth; // for trace, hole
  QWidget *wc; 
  DimSpinner *w; // for pad
  QWidget *hc;
  DimSpinner *h; // for pad
  QWidget *idc;
  DimSpinner *id; // for hole
  QWidget *odc;
  DimSpinner *od; // for hole
  QWidget *squarec;
  QToolButton *circle; // for hole
  QToolButton *square; // for hole

  QWidget *refg;
  QAction *refa;
  QLineEdit *ref;
  QToolButton *component; // popup for replacing component
  
  QWidget *textg;
  QAction *texta;
  DimSpinner *fs; // for text
  QLineEdit *text;

  QWidget *layerg;
  QAction *layera;
  QToolButton *silk;
  QToolButton *top;
  QToolButton *bottom;

  QWidget *orientg;
  QAction *orienta;
  QToolButton *left;
  QToolButton *up;
  QToolButton *right;
  QToolButton *down;
  QToolButton *flip;
  
  bool metric;
public:
  void switchToMetric();
  void switchToInch();
  Dim getDim(DimSpinner *);
  void setDim(DimSpinner *, Dim const &);
  void setupUI();
  void getProperties(); // from editor
  void hideAndShow(); // hide and show as appropriate
  void hsEdit();
};

void PBData::getProperties() {
}

void PBData::hideAndShow() {
  qDebug() << "hideandshow";
  xya->setEnabled(false);
  dima->setEnabled(false);
  refa->setEnabled(false);
  texta->setEnabled(false);
  layera->setEnabled(false);
  orienta->setEnabled(false);

  xc->setEnabled(false);
  yc->setEnabled(false);
  linewidthc->setEnabled(false);
  wc->setEnabled(false);
  hc->setEnabled(false);
  idc->setEnabled(false);
  odc->setEnabled(false);
  squarec->setEnabled(false);

  switch (mode) {
  case Mode::Invalid:
    break;
  case Mode::Edit:
    hsEdit();
    break;
  case Mode::PlaceTrace:
    dima->setEnabled(true);
    linewidthc->setEnabled(true);
    layera->setEnabled(true);
    break;
  case Mode::PlaceComponent:
    layera->setEnabled(true);
    refa->setEnabled(true);
    orienta->setEnabled(true);
    break;
  case Mode::PlaceHole:
    dima->setEnabled(true);
    idc->setEnabled(true);
    odc->setEnabled(true);
    squarec->setEnabled(true);
    break;
  case Mode::PlacePad:
    dima->setEnabled(true);
    wc->setEnabled(true);
    hc->setEnabled(true);
    layera->setEnabled(true);
    break;
  case Mode::PlaceText:
    texta->setEnabled(true);
    layera->setEnabled(true);
    orienta->setEnabled(true);
    break;
  case Mode::PlacePlane:
    layera->setEnabled(true);
    break;
  case Mode::PickupTrace:
    break;
  }
}

void PBData::hsEdit() {
}

void PBData::setupUI() {
  auto makeGroup = [this](QAction **a) {
    Q_ASSERT(parent);
    Q_ASSERT(a);
    QWidget *group = new QWidget;
    auto *lay = new QVBoxLayout;
    lay->setSpacing(8);
    lay->setContentsMargins(0, 0, 0, 12);
    group->setLayout(lay);
    *a = parent->addWidget(group);
    return group;
  };
  
  auto makeContainer = [](QWidget *group) {
    Q_ASSERT(group);
    Q_ASSERT(group->layout());
    QWidget *container = new QWidget(group);
    auto *lay = new QHBoxLayout;
    lay->setSpacing(4);
    lay->setContentsMargins(0, 0, 0, 0);
    container->setLayout(lay);
    group->layout()->addWidget(container);
    return container;
  };

  auto makeDimSpinner = [](QWidget *container,
			   Dim step=Dim::fromInch(.005)) {
    Q_ASSERT(container);
    Q_ASSERT(container->layout());
    DimSpinner *s = new DimSpinner(container);
    s->setStep(step);
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

  /*
  auto makeRadio = [](QWidget *container, QString text) {
    Q_ASSERT(container);
    Q_ASSERT(container->layout());
    QRadioButton *s = new QRadioButton(text);
    container->layout()->addWidget(s);
    return s;
  };
  */

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

  /*
  auto makeCheckBox = [](QWidget *container, QString text) {
    Q_ASSERT(container);
    Q_ASSERT(container->layout());
    QCheckBox *s = new QCheckBox(text);
    container->layout()->addWidget(s);
    return s;
  };
  */
    
  xyg = makeGroup(&xya);
  xc = makeContainer(xyg);
  makeLabel(xc, "X");
  x = makeDimSpinner(xc, Dim::fromInch(.050));
  yc = makeContainer(xyg);
  makeLabel(yc, "Y");
  y = makeDimSpinner(yc, Dim::fromInch(.050));

  dimg = makeGroup(&dima);
  linewidthc = makeContainer(dimg);
  makeIcon(linewidthc, "Width")->setToolTip("Line width");
  linewidth = makeDimSpinner(linewidthc);
  idc = makeContainer(dimg);
  makeLabel(idc, "ID")->setToolTip("Hole diameter");
  id = makeDimSpinner(idc);
  odc = makeContainer(dimg);
  makeLabel(odc, "OD")->setToolTip("Pad diameter");
  od = makeDimSpinner(odc);
  wc = makeContainer(dimg);
  makeLabel(wc, "W");
  w = makeDimSpinner(wc);
  hc = makeContainer(dimg);
  makeLabel(hc, "H");
  h = makeDimSpinner(hc);
  squarec = makeContainer(dimg);
  makeLabel(squarec, "Shape");
  circle = makeTextTool(squarec, "○");
  circle->setToolTip("Round");
  circle->setChecked(true);
  square = makeTextTool(squarec, "□");
  square->setToolTip("Square");

  refg = makeGroup(&refa);
  auto *c5 = makeContainer(refg);
  makeLabel(c5, "Ref.");
  ref = makeEdit(c5);
  component = makeIconTool(c5, "EditComponent");
  component->setToolTip("Choose package");
  component->setCheckable(false);
  
  textg = makeGroup(&texta);
  auto *c1 = makeContainer(textg);
  makeLabel(c1, "Aa")->setToolTip("Text");
  auto *c2 = makeContainer(textg);
  text = makeEdit(c2);
  fs = makeDimSpinner(c1);
  fs->setToolTip("Font size");

  orientg = makeGroup(&orienta);
  auto *c3 = makeContainer(orientg);
  auto *c4 = makeContainer(c3);
  up = makeIconTool(c4, "Up");
  right = makeIconTool(c4, "Right");
  down = makeIconTool(c4, "Down");
  left = makeIconTool(c4, "Left");
  flip = makeIconTool(c3, "Flip");

  layerg = makeGroup(&layera);
  auto *lc = makeContainer(layerg);
  makeLabel(lc, "Layer");
  silk = makeIconTool(lc, "Silk");
  top = makeIconTool(lc, "Top");
  bottom = makeIconTool(lc, "Bottom");
}  

Propertiesbar::Propertiesbar(Editor *editor, QWidget *parent): QToolBar(parent) {
  d = new PBData;
  d->parent = this;
  d->editor = editor;
  d->metric = false;
  d->mode = Mode::Edit;
  d->setupUI();
}

void Propertiesbar::reflectMode(Mode m) {
  d->mode = m;
  d->hideAndShow();
}

void Propertiesbar::reflectSelection() {
  d->getProperties();
  d->hideAndShow();
}

void Propertiesbar::reflectBoard(class Board const &b) {
  bool m = b.isEffectivelyMetric();
  d->metric = m;
  d->x->setMetric(m);
  d->y->setMetric(m);
  d->w->setMetric(m);
  d->h->setMetric(m);
  d->id->setMetric(m);
  d->od->setMetric(m);
  d->linewidth->setMetric(m);
  d->fs->setMetric(m);
}

void Propertiesbar::forwardAllProperties() {
  if (!d->editor)
    return;
  d->editor->setWidth(Dim());
}
