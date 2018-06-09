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
#include "data/Object.h"

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
  QToolButton *silk, *top, *bottom;

  QWidget *orientg;
  QAction *orienta;
  QWidget *orientc;
  QWidget *rotatec;
  QToolButton *ccw, *cw;
  QToolButton *left, *up, *right, *down;
  QToolButton *flipped;
  QToolButton *fliph, *flipv;
  
  bool metric;
  
  Dim x0, y0;
public:
  void switchToMetric();
  void switchToInch();
  Dim getDim(DimSpinner *);
  void setDim(DimSpinner *, Dim const &);
  void setupUI();
  void getPropertiesFromSelection(); // from editor
  void hideAndShow(); // hide and show as appropriate
  void hsEdit();
  Layer layer() const;
};

void PBData::getPropertiesFromSelection() {
  QSet<int> objects(editor->selectedObjects());
  QSet<Point> points(editor->selectedPoints());
  Group const &here(editor->currentGroup());

  if (!points.isEmpty()) {
    x0 = Dim::fromInch(1000);
    y0 = Dim::fromInch(1000);
    for (Point const &p: points) {
      if (p.x<x0)
	x0 = p.x;
      if (p.y<y0)
	y0 = p.y;
    }
    x->setValue(x0);
    y->setValue(y0);
  }

  bool got = false;
  // Set linewidth if we have at least one trace and their widths are all same
  for (int k: objects) {
    if (here.object(k).isTrace()) {
      Dim lw = here.object(k).asTrace().width;
      if (got) {
	if (lw != linewidth->value())
	  linewidth->setNoValue();
      } else {
	linewidth->setValue(lw);
	got = true;
      }
    }
  }
    
  got = false;
  // Set w (h) if we have at least one pad and their widths (heights) are
  // all same
  for (int k: objects) {
    if (here.object(k).isPad()) {
      Dim w1 = here.object(k).asPad().width;
      Dim h1 = here.object(k).asPad().height;
      if (got) {
	if (w1 != w->value())
	  w->setNoValue();
	if (h1 != h->value())
	  h->setNoValue();
      } else {
	w->setValue(w1);
	h->setValue(h1);
	got = true;
      }
    }
  }

  got = false;
  // Set id (od, square) if we have at least one hole and their id (etc)
  // etc all same
  square->setChecked(false);
  circle->setChecked(false);
  for (int k: objects) {
    if (here.object(k).isHole()) {
      Dim id1 = here.object(k).asHole().id;
      Dim od1 = here.object(k).asHole().od;
      bool sq = here.object(k).asHole().square;
      if (got) {
	if (id1 != id->value())
	  id->setNoValue();
	if (od1 != od->value())
	  od->setNoValue();
	if (sq && circle->isChecked())
	  circle->setChecked(false);
	else if (!sq && square->isChecked())
	  square->setChecked(false);
      } else {
	id->setValue(id1);
	od->setValue(od1);
	if (sq)
	  square->setChecked(true);
	else
	  circle->setChecked(true);
	got = true;
      }
    }
  }
  // more: text, ref
}

Layer PBData::layer() const {
  if (silk->isChecked())
    return Layer::Silk;
  else if (top->isChecked())
    return Layer::Top;
  else if (bottom->isChecked())
    return Layer::Bottom;
  else
    return Layer::Invalid;
}

void PBData::hideAndShow() {
  qDebug() << "hs" << int(mode);
  
  xya->setEnabled(false);
  dima->setEnabled(false);
  refa->setEnabled(false);
  texta->setEnabled(false);
  layera->setEnabled(false);
  orienta->setEnabled(false);

  linewidthc->setEnabled(false);
  wc->setEnabled(false);
  hc->setEnabled(false);
  idc->setEnabled(false);
  odc->setEnabled(false);
  squarec->setEnabled(false);
  orientc->setVisible(true);
  flipped->setVisible(true);
  rotatec->setVisible(false);

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
  orienta->setEnabled(true);
  orientc->setVisible(false);
  flipped->setVisible(false);
  rotatec->setVisible(true);
  
  QSet<int> objects(editor->selectedObjects());
  QSet<Point> points(editor->selectedPoints());
  Group const &here(editor->currentGroup());

  // Now, what do we enable?
  // Show X,Y if anything at all selected
  xya->setEnabled(!objects.isEmpty() || !points.isEmpty());

  // Show linewidth if we have at least one trace
  for (int k: objects) {
    if (here.object(k).isTrace()) {
      dima->setEnabled(true);
      linewidthc->setEnabled(true);
      break;
    }
  }

  // Show width, height if we have at least one pad
  for (int k: objects) {
    if (here.object(k).isPad()) {
      dima->setEnabled(true);
      wc->setEnabled(true);
      hc->setEnabled(true);
      break;
    }
  }

  // Show id, od, sq. if we have at least one hole
  for (int k: objects) {
    if (here.object(k).isHole()) {
      dima->setEnabled(true);
      idc->setEnabled(true);
      odc->setEnabled(true);
      squarec->setEnabled(true);
      break;
    }
  }

  // Show ref if we have exactly one hole or exactly one group
  if (objects.size()==1) {
    for (int k: objects) { // loop of 1, but each to write this way
      if (here.object(k).isGroup()
	  || here.object(k).isHole()
	  || here.object(k).isPad())
	refa->setEnabled(true);
    }
  }

  // Show text if we have exactly one text; font size if we have at
  // least one text
  for (int k: objects) {
    if (here.object(k).isText()) {
      texta->setEnabled(true);
      text->setEnabled(objects.size()==1);
      break;
    }
  }
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

  auto makeTextTool = [](QWidget *container, QString text, bool excl=true) {
    Q_ASSERT(container);
    Q_ASSERT(container->layout());
    QToolButton *s = new QToolButton;
    s->setText(text);
    s->setCheckable(true);
    s->setAutoExclusive(excl);
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
  linewidth->setValue(Dim::fromInch(.010));
  QObject::connect(linewidth, &DimSpinner::valueEdited,
		   [this](Dim d) { editor->setLineWidth(d); });
  idc = makeContainer(dimg);
  makeLabel(idc, "ID")->setToolTip("Hole diameter");
  id = makeDimSpinner(idc);
  id->setMinimumValue(Dim::fromInch(0.005));
  id->setValue(Dim::fromInch(.040));
  QObject::connect(id, &DimSpinner::valueEdited,
		   [this](Dim d) {
		     if (od->hasValue()
			 && (od->value() < d + Dim::fromInch(0.015)))
		       od->setValue(d + Dim::fromInch(0.015), true);
		     editor->setID(d); });
  odc = makeContainer(dimg);
  makeLabel(odc, "OD")->setToolTip("Pad diameter");
  od = makeDimSpinner(odc);
  od->setMinimumValue(Dim::fromInch(0.020));
  od->setValue(Dim::fromInch(.065));
  QObject::connect(od, &DimSpinner::valueEdited,
		   [this](Dim d) {
		     if (id->hasValue()
			 && (id->value() > d - Dim::fromInch(0.015)))
		       id->setValue(d - Dim::fromInch(0.015), true);
		     editor->setOD(d);
		   });
  wc = makeContainer(dimg);
  makeLabel(wc, "W");
  w = makeDimSpinner(wc);
  w->setValue(Dim::fromInch(.020));
  w->setMinimumValue(Dim::fromInch(0.005));
  hc = makeContainer(dimg);
  makeLabel(hc, "H");
  h = makeDimSpinner(hc);
  h->setValue(Dim::fromInch(.040));
  h->setMinimumValue(Dim::fromInch(0.005));
  squarec = makeContainer(dimg);
  makeLabel(squarec, "Shape");
  circle = makeTextTool(squarec, "○", false);
  circle->setToolTip("Round");
  circle->setChecked(true);
  square = makeTextTool(squarec, "□", false);
  square->setToolTip("Square");
  QObject::connect(square, &QToolButton::clicked,
		   [this](bool b) {
		     if (b) {
		       circle->setChecked(false);
		       editor->setSquare(true);
		     }
		   });
  QObject::connect(circle, &QToolButton::clicked,
		   [this](bool b) {
		     if (b) {
		       square->setChecked(false);
		       editor->setSquare(false);
		     }
		   });

  refg = makeGroup(&refa);
  auto *c5 = makeContainer(refg);
  makeLabel(c5, "Ref.");
  ref = makeEdit(c5);
  QObject::connect(ref, &QLineEdit::textEdited,
		   [this](QString txt) { editor->setRef(txt); });
  component = makeIconTool(c5, "EditComponent");
  component->setToolTip("Choose package");
  component->setCheckable(false);
  
  textg = makeGroup(&texta);
  auto *c1 = makeContainer(textg);
  makeLabel(c1, "Aa")->setToolTip("Text");
  auto *c2 = makeContainer(textg);
  text = makeEdit(c2);
  QObject::connect(ref, &QLineEdit::textEdited,
		   [this](QString txt) { editor->setText(txt); });
  fs = makeDimSpinner(c1);
  fs->setValue(Dim::fromInch(.050));
  fs->setToolTip("Font size");

  orientg = makeGroup(&orienta);
  auto *c3 = makeContainer(orientg);
  orientc = makeContainer(c3);
  rotatec = makeContainer(c3);
  up = makeIconTool(orientc, "Up");
  right = makeIconTool(orientc, "Right");
  down = makeIconTool(orientc, "Down");
  left = makeIconTool(orientc, "Left");
  ccw = makeIconTool(rotatec, "CCW");
  ccw->setToolTip("Rotate left");
  ccw->setCheckable(false);
  cw = makeIconTool(rotatec, "CW");
  ccw->setToolTip("Rotate right");
  cw->setCheckable(false);
  fliph = makeIconTool(rotatec, "FlipH");
  ccw->setToolTip("Flip left to right");
  fliph->setCheckable(false);
  ccw->setToolTip("Flip top to bottom");
  flipv = makeIconTool(rotatec, "FlipV");
  flipv->setCheckable(false);
  flipped = makeIconTool(c3, "Flipped");

  layerg = makeGroup(&layera);
  auto *lc = makeContainer(layerg);
  makeLabel(lc, "Layer");
  silk = makeIconTool(lc, "Silk");
  QObject::connect(silk, &QToolButton::clicked,
		   [this]() { editor->setLayer(Layer::Silk); });
  top = makeIconTool(lc, "Top");
  QObject::connect(top, &QToolButton::clicked,
		   [this]() { editor->setLayer(Layer::Top); });
  bottom = makeIconTool(lc, "Bottom");
  QObject::connect(bottom, &QToolButton::clicked,
		   [this]() { editor->setLayer(Layer::Bottom); });
  top->setChecked(true);
}  

Propertiesbar::Propertiesbar(Editor *editor, QWidget *parent): QToolBar(parent) {
  d = new PBData;
  d->parent = this;
  d->editor = editor;
  d->metric = false;
  d->mode = Mode::Edit;
  d->setupUI();
  connect(editor, &Editor::selectionChanged,
	  this, &Propertiesbar::reflectSelection);
}

void Propertiesbar::reflectMode(Mode m) {
  d->mode = m;
  if (m==Mode::PlaceHole) {
    if (!d->square->isChecked())
      d->circle->setChecked(true);
  }
  d->hideAndShow();
}

void Propertiesbar::reflectSelection() {
  qDebug() << "reflectselection";
  d->getPropertiesFromSelection();
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
  d->editor->setLineWidth(d->linewidth->value());
  d->editor->setLayer(d->layer());
  d->editor->setID(d->id->value());
  d->editor->setOD(d->od->value());
  d->editor->setSquare(d->square->isChecked());
}
