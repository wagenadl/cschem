// Statusbar.cpp

#include "Statusbar.h"
#include <QAction>
#include <QComboBox>
#include <QLabel>
#include <QToolButton>

Statusbar::Statusbar(QWidget *parent): QStatusBar(parent) {
  //auto *w = new QToolButton;
  //w->setIcon(QIcon(":icons/Grid.svg"));
  //addWidget(w);

  cursorui = new QLabel("X:0.100” Y:0.900”");
  addWidget(cursorui);
  
  gridui = new QComboBox;
  gridui->setToolTip("Grid spacing");
  gridui->addItem("Off", QVariant(Dim().toString()));
  gridui->addItem("0.025”", QVariant(Dim::fromMils(25).toString()));
  gridui->addItem("0.050”", QVariant(Dim::fromMils(50).toString()));
  gridui->addItem("0.100”", QVariant(Dim::fromMils(100).toString()));
  gridui->addItem("M0.5", QVariant(Dim::fromMM(.5).toString()));  
  gridui->addItem("M1.0", QVariant(Dim::fromMM(1).toString()));  
  gridui->addItem("M2.0", QVariant(Dim::fromMM(2).toString()));
  addWidget(gridui);

  auto addlui = [this](Layer l, QString name) {
    auto *w = new QToolButton;
    layerui[l] = w;
    w->setIcon(QIcon(":icons/" + name + ".svg"));
    w->setToolTip(name);
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
  return Dim();
}

bool Statusbar::isLayerVisible(Layer) const {
  return false;
}

bool Statusbar::arePlanesVisible() const {
  return false;
}

void Statusbar::setGrid(Dim) {
}

void Statusbar::hideLayer(Layer) {
}

void Statusbar::showLayer(Layer) {
}

void Statusbar::hidePlanes() {
}

void Statusbar::showPlanes() {
}

void Statusbar::setCursorXY(Point) {
}
