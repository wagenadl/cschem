// Statusbar.cpp

#include "Statusbar.h"
#include <QAction>
#include <QComboBox>
#include <QLabel>
#include <QToolButton>

Statusbar::Statusbar(QWidget *parent): QStatusBar(parent) {

  cursorui = new QLabel();
  hideCursorXY();
  addWidget(cursorui);

  auto *w1 = new QLabel;
  w1->setPixmap(QIcon(":icons/Grid.svg").pixmap(cursorui->height()));
  w1->setTooltip("Grid spacing");
  addWidget(w1);
  gridui = new QComboBox;
  gridui->setToolTip("Grid spacing");
  resetGridChoices();
  addWidget(gridui);

  auto addlui = [this](Layer l, QString name) {
    auto *w = new QToolButton;
    layerui[l] = w;
    w->setIcon(QIcon(":icons/" + name + ".svg"));
    w->setToolTip(name + " layer visible");
    w->setCheckable(true);
    w->setChecked(true);
    connect(w, &QToolButton::triggered,
	    [this, l]() { layerVisibilityEdited(l, layerui[l]->isChecked()); });
    addWidget(w);
  };
  addlui(Layer::Silk, "Silk");
  addlui(Layer::Top, "Top");
  addlui(Layer::Bottom, "Bottom");

  auto *w = new QToolButton;
  planesui = w;
  w->setIcon(QIcon(":icons/Planes.svg"));
  w->setToolTip("Planes visible");
  w->setCheckable(true);
  w->setChecked(true);
  connect(w, &QToolButton::triggered,
	  [this]() { planesVisibilityEdited(planesui->isChecked()); });
  addWidget(w);
  
}

Statusbar::~Statusbar() {
}

Dim Statusbar::gridSpacing() const {
  return Dim::fromString(gridui->currentData().toString());
}

bool Statusbar::isLayerVisible(Layer) const {
  return false;
}

bool Statusbar::arePlanesVisible() const {
  return false;
}

void Statusbar::setBoard(Board const &b) {
  board = b;
  resetGridChoices();
  setGrid(b.grid);
}

void Statusbar::resetGridChoices() {
  gridui->clear();
  gridui->addItem("Off", QVariant(Dim().toString()));
  gridui->addItem("0.025”", QVariant(Dim::fromMils(25).toString()));
  gridui->addItem("0.050”", QVariant(Dim::fromMils(50).toString()));
  gridui->addItem("0.100”", QVariant(Dim::fromMils(100).toString()));
  gridui->addItem("0.5 mm", QVariant(Dim::fromMM(.5).toString()));  
  gridui->addItem("1.0 mm", QVariant(Dim::fromMM(1).toString()));  
  gridui->addItem("2.0 mm", QVariant(Dim::fromMM(2).toString()));
}  

void Statusbar::setGrid(Dim g) {
  int idx = gridui->findData(QVariant(g.toString()));
  if (idx>=0) {
    gridui->setCurrentIndex(idx);
  } else  {
    if (board.metric)
      gridui->addItem(QString("%1 mm").arg(g.toMM()),
		      QVariant(g.toString()));
    else
      gridui->addItem(QString("%1”").arg(g.toInch()),
		      QVariant(g.toString()));
    gridui->setCurrentIndex(gridui->count()-1);
  }
}

void Statusbar::hideLayer(Layer) {
}

void Statusbar::showLayer(Layer) {
}

void Statusbar::hidePlanes() {
}

void Statusbar::showPlanes() {
}

void Statusbar::setCursorXY(Point p) {
  if (p.x>=Dim() && p.y>=Dim() && p.x<=board.width && p.y<=board.height) {
    bool metric = board.grid.isNull() ? board.metric : board.grid.isMetric();
    if (metric) 
      cursorui->setText(QString("X:%1 Y:%2")
			.arg(p.x.toInch(),5,'f',2, QChar(0x2007))
			.arg(p.y.toInch(),5,'f',2, QChar(0x2007)));
    else
      cursorui->setText(QString("X:%1 Y:%2")
			.arg(p.x.toInch(),0,'f',3)
			.arg(p.y.toInch(),0,'f',3));
  } else {
    hideCursorXY();
  }
}

void Statusbar::hideCursorXY() {
  bool metric = board.grid.isNull() ? board.metric : board.grid.isMetric();
  if (metric) 
    cursorui->setText("X:‒‒.‒‒ Y:‒‒.‒‒");
  else
    cursorui->setText("X:‒.‒‒‒ Y:‒.‒‒‒");
}
