// Statusbar.cpp

#include "Statusbar.h"
#include  "DimSpinner.h"
#include <QAction>
#include <QComboBox>
#include <QLabel>
#include <QToolButton>

Statusbar::Statusbar(QWidget *parent): QStatusBar(parent) {
  noemit = true;
  cursorui = new QLabel();
  cursorui->setStyleSheet("QLabel { margin-left: 4px; }");
  missingui = new QToolButton();
  missingui->setIcon(style()->standardIcon(QStyle::SP_MessageBoxWarning));
  missingui->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
  hideCursorXY();
  missingui->hide();
  addWidget(cursorui);
  addWidget(missingui);
  setStyleSheet("QToolButton:checked { padding: 2px; margin: 2px; background-color: #ffffcc;}");
  setSizeGripEnabled(false);

  auto *w1 = new QLabel;
  w1->setPixmap(QIcon(":icons/Grid.svg").pixmap(cursorui->height()));
  w1->setToolTip("Grid spacing");
  addPermanentWidget(w1);
  gridsp = new DimSpinner;
  gridsp->setMode(Expression::Mode::Explicit);
  gridsp->hideTrailingZeros();
  gridui = new QComboBox;
  gridui->setStyleSheet("QComboBox { margin-right: 8px; }");
  gridui->setEditable(true);
  gridui->setInsertPolicy(QComboBox::InsertAtBottom);
  gridui->setLineEdit(gridsp);
  gridui->setToolTip("Grid spacing");
  connect(gridui, QOverload<QString const &>::of(&QComboBox::activated),
	  [this]() {
	    int mc = gridui->maxCount();
	    gridsp->parseValue();
	    if (gridui->currentIndex()==mc-1) {
	      if (gridsp->isValid()) 
		gridui->removeItem(mc-2);
	      else
		gridui->removeItem(mc-1);
	    }
	    gridsp->parseValue();
	    qDebug() << "gridsp" << gridsp->isValid() << gridsp->hasValue() << gridsp->value() << gridsp->isMetric();
	    if (gridsp->isValid()) {
	      Dim g = gridsp->value();
	      gridui->setItemData(gridui->currentIndex(),
				  QVariant(g.toString()));
	      if (g != board.grid) {
		board.grid = g;
		if (!noemit)
		  emit gridEdited(board.grid);
	      }
	    }
	  });
  resetGridChoices();
  addPermanentWidget(gridui);

  auto addlui = [this](Layer l, QString name, QKeySequence seq) {
    auto *w = new QToolButton;
    layerui[l] = w;
    w->setIcon(QIcon(":icons/" + name + ".svg"));
    w->setToolTip(name + " layer visible (" + seq.toString() + ")");
    w->setCheckable(true);
    w->setChecked(true);
    w->setShortcut(seq);
    connect(w, &QToolButton::toggled,
	    [this, l]() { layerVisibilityEdited(l, layerui[l]->isChecked()); });
    addPermanentWidget(w);
  };
  addlui(Layer::Silk, "Silk", QKeySequence(Qt::Key_1));
  addlui(Layer::Top, "Top", QKeySequence(Qt::Key_2));
  addlui(Layer::Bottom, "Bottom", QKeySequence(Qt::Key_3));
  addlui(Layer::Panel, "Panel", QKeySequence(Qt::Key_4));

  auto *w = new QToolButton;
  planesui = w;
  w->setIcon(QIcon(":icons/Planes.svg"));
  w->setToolTip("Planes visible (5)");
  w->setShortcut(QKeySequence(Qt::Key_5));
  w->setCheckable(true);
  w->setChecked(true);
  connect(w, &QToolButton::toggled,
	  [this]() { planesVisibilityEdited(planesui->isChecked()); });
  addPermanentWidget(w);

  w = new QToolButton;
  netsui = w;
  w->setIcon(QIcon(":icons/Nets.svg"));
  w->setToolTip("Nets visible (6)");
  w->setShortcut(QKeySequence(Qt::Key_6));
  w->setCheckable(true);
  w->setChecked(true);
  connect(w, &QToolButton::toggled,
	  [this]() { netsVisibilityEdited(netsui->isChecked()); });
  w->setStyleSheet("QToolButton { margin-right: 8px; }");
  addPermanentWidget(w);

  
  noemit = false;
}

Statusbar::~Statusbar() {
}

Dim Statusbar::gridSpacing() const {
  return Dim::fromString(gridui->currentData().toString());
}

bool Statusbar::isLayerVisible(Layer l) const {
  return layerui.contains(l) ? layerui[l]->isChecked() : true;
}

bool Statusbar::arePlanesVisible() const {
  return planesui->isChecked();
}

bool Statusbar::areNetsVisible() const {
  return netsui->isChecked();
}

void Statusbar::setBoard(Board const &b) {
  board = b;
  noemit = true;
  resetGridChoices();
  setGrid(b.grid);
  planesui->setChecked(b.planesvisible);
  layerui[Layer::Panel]->setChecked(b.layervisible[Layer::Panel]);
  layerui[Layer::Silk]->setChecked(b.layervisible[Layer::Silk]);
  layerui[Layer::Top]->setChecked(b.layervisible[Layer::Top]);
  layerui[Layer::Bottom]->setChecked(b.layervisible[Layer::Bottom]);
  noemit = false;
}

void Statusbar::resetGridChoices() {
  Dim g = board.grid;
  gridui->clear();
  gridsp->setNoValueText("Off");
  gridui->addItem("Off", QVariant(Dim().toString()));
  gridui->addItem("0.025”", QVariant(Dim::fromMils(25).toString()));
  gridui->addItem("0.050”", QVariant(Dim::fromMils(50).toString()));
  gridui->addItem("0.100”", QVariant(Dim::fromMils(100).toString()));
  gridui->addItem("0.5 mm", QVariant(Dim::fromMM(.5).toString()));  
  gridui->addItem("1.0 mm", QVariant(Dim::fromMM(1).toString()));  
  gridui->addItem("2.0 mm", QVariant(Dim::fromMM(2).toString()));
  gridui->setMaxCount(9);
  int idx = gridui->findData(QVariant(g.toString()));
  if (idx>=0) {
    gridui->setCurrentIndex(idx);
  } else {
    gridui->setCurrentIndex(7);
    if (g.isMetric())
      gridui->setItemText(7, QString("%1 mm").arg(g.toMM()));
    else
      gridui->setItemText(7, QString("%1”").arg(g.toInch()));
    gridsp->parseValue();
  }
}

void Statusbar::setGrid(Dim g) {
  board.grid = g;
  qDebug() << "setgrid" << g;
  int idx = gridui->findData(QVariant(g.toString()));
  qDebug() << "idx = " << idx;
  qDebug() << "mc = " << gridui->maxCount() << gridui->currentIndex();
  if (idx>=0) {
    gridui->setCurrentIndex(idx);
  } else {
    for (int i=0; i<gridui->count(); i++) {
      qDebug() << "at" << i << gridui->itemText(i) << gridui->itemData(i);
    }
    if (g.isMetric())
      gridui->addItem(QString("%1 mm").arg(g.toMM()),
		      QVariant(g.toString()));
    else
      gridui->addItem(QString("%1”").arg(g.toInch()),
		      QVariant(g.toString()));
    gridui->setCurrentIndex(gridui->count()-1);
    gridsp->parseValue();
  }
}

void Statusbar::hideLayer(Layer l) {
  board.layervisible[l] = false;
  if (layerui.contains(l))
    layerui[l]->setChecked(false);
}

void Statusbar::showLayer(Layer l) {
  board.layervisible[l] = true;
  if (layerui.contains(l))
    layerui[l]->setChecked(true);
}

void Statusbar::hidePlanes() {
  board.planesvisible = false;
  planesui->setChecked(false);
}

void Statusbar::showPlanes() {
  board.planesvisible = true;
  planesui->setChecked(true);
}

void Statusbar::hideNets() {
  netsui->setChecked(false);
}

void Statusbar::showNets() {
  netsui->setChecked(true);
}

void Statusbar::setCursorXY(Point p1) {
  p = p1;
  updateCursor();
}

void Statusbar::setObject(QString obj1) {
  obj = obj1;
  updateCursor();
}

void Statusbar::setMissing(QStringList mis1) {
  if (mis1.isEmpty()) {
    missingui->hide();
  } else {
    if (mis1.size() > 5) {
      while (mis1.size()>5)
	mis1.removeLast();
      mis1 << "…";
    }
    missingui->setText("Missing " + mis1.join(", "));
    missingui->show();
  }
}

void Statusbar::setUserOrigin(Point o) {
  ori = o;
  updateCursor();
}

void Statusbar::updateCursor() {
  if (p.x>=Dim() && p.y>=Dim() && p.x<=board.width && p.y<=board.height) {
    Point p1 = p - ori;
    bool metric = board.isEffectivelyMetric();
    QString txt;
    if (metric) 
      txt = QString("X:%1 mm Y:%2 mm")
	.arg(p1.x.toMM(),5,'f',2, QChar(0x2007))
	.arg(p1.y.toMM(),5,'f',2, QChar(0x2007));
    else
      txt = QString("X:%1” Y:%2”")
	.arg(p1.x.toInch(),0,'f',3)
	.arg(p1.y.toInch(),0,'f',3);
    if (!obj.isEmpty())
      txt += " on " + obj;
    txt.replace("-", "−");
    cursorui->setText(txt);
  } else {
    hideCursorXY();
  }
}

void Statusbar::hideCursorXY() {
  bool metric = board.isEffectivelyMetric();
  if (metric) 
    cursorui->setText("X:‒‒.‒‒ Y:‒‒.‒‒");
  else
    cursorui->setText("X:‒.‒‒‒ Y:‒.‒‒‒");
}
