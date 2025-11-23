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
#include <QTextEdit>
#include "data/Object.h"
#include "AlignToggle.h"
#include "AbsIncToggle.h"
#include <QSpinBox>

const Dim minRingWidth(Dim::fromMM(0.3));

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
  AlignToggle *xalign;
  AlignToggle *yalign;
  AbsIncToggle *xabsinc;
  AbsIncToggle *yabsinc;
  
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
  QWidget *slotlengthc; // for hole
  DimSpinner *slotlength;
  QWidget *squarec;
  QAction *circle; // for hole
  QAction *square; // for hole
  QAction *via;

  QWidget *textg;
  QAction *texta;
  QLabel *textl;
  QLineEdit *text;
  DimSpinner *fs;

  QWidget *arcg;
  QAction *arca;
  QWidget *arcc;
  QAction *arc360, *arc300, *arc240, *arc180, *arc120, *arc_90;
  
  QWidget *layerg;
  QAction *layera;
  QAction *panel, *silk, *bsilk, *top, *bottom;

  QWidget *orientg;
  QAction *orienta;
  QWidget *orientc;
  QWidget *rotatec;
  QAction *ccw, *cw;
  QAction *left, *up, *right, *down;
  QAction *flipped;
  QAction *fliph, *flipv;

  QWidget *grouppropg;
  QAction *grouppropa;
  QLineEdit *pkg;
  QSpinBox *rot;
  QTextEdit *notes;
  
  bool metric;
  
  Dim x0, y0;
  Point p0;
  Rect r0;
  Point ori;
  QMap<QWidget *, QSet<QToolButton *>> exclusiveGroups;
public:
  PBData() { xya=0; }
  void switchToMetric();
  void switchToInch();
  Dim getDim(DimSpinner *);
  void setDim(DimSpinner *, Dim const &);
  void setupUI();
  void getPropertiesFromSelection(); // from editor
  void hideAndShow(); // hide and show as appropriate
  void hsEdit();
  Layer checkedLayer() const;
  int arcAngle() const;
  void checkLayer(Layer);
  bool anyLayerChecked() const;
  bool anyDirectionChecked() const;
private:
  void getRectFromObjectsAndPoints(QSet<int> const &objects, Group const &here,
                                   QSet<Point> const &points);
  bool getP0fromPoints(QSet<Point> const &points);
  bool getP0fromGroup(QSet<int> const &objects, Group const &here);
  bool getP0fromText(QSet<int> const &objects, Group const &here);
  bool getP0fromOrigin();
  void updateAlignment();
  void fillLinewidth(QSet<int> const &objects, Group const &here);
  void fillWH(QSet<int> const &objects, Group const &here);
  void fillDiamAndShape(QSet<int> const &objects, Group const &here);
  void fillRefText(QSet<int> const &objects, Group const &here);
  void fillArcAngle(QSet<int> const &objects, Group const &here);
  void fillLayer(QSet<int> const &objects, Group const &here);
  void fillFontSize(QSet<int> const &objects, Group const &here);    
  void fillGroupProps(QSet<int> const &objects, Group const &here);
  void clickAbsInc(Qt::Orientation ori);
public:
  void doFillArcAngle(int arcangle);
};

void PBData::checkLayer(Layer l) {
  panel->setChecked(l==Layer::Panel);
  silk->setChecked(l==Layer::Silk);
  bsilk->setChecked(l==Layer::BSilk);
  top->setChecked(l==Layer::Top);
  bottom->setChecked(l==Layer::Bottom);
}

bool PBData::anyDirectionChecked() const {
  return up->isChecked() || down->isChecked()
    || right->isChecked() || left->isChecked();
}

bool PBData::anyLayerChecked() const {
  return panel->isChecked()
    || silk->isChecked() || bsilk->isChecked()
    || top->isChecked() || bottom->isChecked();
}

void PBData::getRectFromObjectsAndPoints(QSet<int> const &objects, Group const &here,
                                        QSet<Point> const &points) {
  r0 = Rect();
  for (int id: objects)
    r0 |= here.object(id).boundingRect();
  for (Point const &p: points)
    r0 |= p;
}
      

bool PBData::getP0fromGroup(QSet<int> const &objects, Group const &here) {
  // only works if OBJECTS contains only a group, possibly with its reftext
  if (objects.size() > 2)
    return false;
  for (int id: objects) {
    if (here.object(id).isGroup()) {
      Group const &group(here.object(id).asGroup());
      if (objects.size()>=2) {
        if (group.refTextId()<=0 || !objects.contains(group.refTextId()))
          return false;
      }
      p0 = group.anchor();
      return true;
    }
  }
  return false;
}
      

bool PBData::getP0fromPoints(QSet<Point> const &points) {
  if (points.isEmpty())
    return false;
  Dim x = Dim::fromInch(1000);
  Dim y = Dim::fromInch(1000);
  for (Point const &p: points) {
    if (p.x < x)
      x = p.x;
    if (p.y < y)
      y = p.y;
  }
  p0 = Point(x, y);
  return true;
}

bool PBData::getP0fromText(QSet<int> const &objects,
                           Group const &here) {
  bool got = false;
  Dim x = Dim::fromInch(1000);
  Dim y = Dim::fromInch(1000);
  for (int k: objects) {
    Object const &obj(here.object(k));
    if (obj.isText()) {
      Text const &text(obj.asText());
      if (text.p.x < x)
        x = text.p.x;
      if (text.p.y < y)
        y = text.p.y;
      got = true;
    }
  }
  if (got)
    p0 = Point(x, y);
  return got;
}

bool PBData::getP0fromOrigin() {
  p0 = ori;
  return true;
}


void Propertiesbar::reflectTentativeMove(Point dp) {
  d->x->setValue(d->x0 - d->ori.x + dp.x);
  d->y->setValue(d->y0 - d->ori.y + dp.y);
}

void Propertiesbar::setUserOrigin(Point o) {
  if (!d->x->isEnabled()) 
    d->x0 = o.x;
  if (!d->y->isEnabled()) 
    d->y0 = o.y;
  d->ori = o;
  d->x->setValue(d->x0 - o.x);
  d->y->setValue(d->y0 - o.y);
  emit userOriginChanged(o);
}

Point Propertiesbar::userOrigin() const {
  return d->ori;
}

void PBData::fillLinewidth(QSet<int> const &objects, Group const &here) {
  bool got = false;
  // Set linewidth if we have at least one trace/arc and their widths
  // are all same
  for (int k: objects) {
    Object const &obj(here.object(k));
    if (obj.isTrace() || obj.isArc()) {
      Dim lw = obj.isArc() ? obj.asArc().linewidth : obj.asTrace().width;
      if (got) {
	if (lw != linewidth->value())
	  linewidth->setNoValue();
      } else {
	linewidth->setValue(lw);
	got = true;
      }
    } 
  }
  
  if (linewidth->hasValue())
    editor->properties().linewidth = linewidth->value();
}

void PBData::fillWH(QSet<int> const &objects, Group const &here) {
  bool got = false;
  // Set w (h) if we have at least one pad and their widths (heights) are
  // all same
  for (int k: objects) {
    Object const &obj(here.object(k));
    if (obj.isPad()) {
      Dim w1 = obj.asPad().width;
      Dim h1 = obj.asPad().height;
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
  if (w->hasValue())
    editor->properties().w = w->value();
  if (h->hasValue())
    editor->properties().h = h->value();
}

void PBData::fillDiamAndShape(QSet<int> const &objects, Group const &here) {
  //qDebug() << "filldiamandshape" << objects << circle->isChecked() << square->isChecked();
  bool got = false;
  // Set id (od, square) if we have at least one hole and their id (etc)
  // etc all same
  for (int k: objects) {
    Object const &obj(here.object(k));
    if (obj.isHole()) {
      Dim id1 = obj.asHole().id;
      Dim od1 = obj.asHole().od;
      Dim sl1 = obj.asHole().slotlength;
      bool sq = obj.asHole().square;
      bool vi = obj.asHole().via;
      if (got) {
	if (id1 != id->value())
	  id->setNoValue();
	if (od1 != od->value())
	  od->setNoValue();
	if (sl1 != slotlength->value())
	  slotlength->setNoValue();
	if (sq && circle->isChecked())
	  circle->setChecked(false);
	else if (!sq && square->isChecked())
	  square->setChecked(false);
        if (vi != via->isChecked()) 
          via->setEnabled(false);
      } else {
	id->setValue(id1);
	od->setValue(od1);
	slotlength->setValue(sl1);
        square->setChecked(sq);
        circle->setChecked(!sq);
        via->setEnabled(true);
        via->setChecked(vi);
	got = true;
      }
    } else if (obj.isNPHole()) {
      Dim id1 = obj.asNPHole().d;
      Dim sl1 = obj.asNPHole().slotlength;
      if (got) {
        if (id1 != id->value())
          id->setNoValue();
	if (sl1 != slotlength->value())
	  slotlength->setNoValue();
      } else {
        id->setValue(id1);
	slotlength->setValue(sl1);
        got = true;
      }
    }
  }
  if (got) {
    // unset ID if we have an arc that doesn't match
    for (int k: objects) {
      Object const &obj(here.object(k));
      if (obj.isArc()) {
	if (obj.asArc().radius != id->value()/2) {
	  id->setNoValue();
	  break;
	}
      }
    }
  } else {
    // set ID if we have arcs and they are all same size
    for (int k: objects) {
      Object const &obj(here.object(k));
      if (obj.isArc()) {
	if (got) {
	  if (obj.asArc().radius != id->value()/2) {
	    id->setNoValue();
	    break;
	  }
	} else {
	  id->setValue(obj.asArc().radius*2);
	  got = true;
	}
      }
    }
  }
  if (id->hasValue())
    editor->properties().id = id->value();
  if (od->hasValue())
    editor->properties().od = od->value();
  if (square->isChecked())
    editor->properties().square = true;
  if (circle->isChecked())
    editor->properties().square = false;
}

void PBData::fillRefText(QSet<int> const &objects, Group const &here) {
  // set ref if we have precisely one hole or group, otherwise clear it
  bool got = false;
  text->setText("");
  int kignore = -1;
  for (int k: objects) {
    Object const &obj(here.object(k));
    if (obj.isHole() || obj.isPad() || obj.isGroup() || obj.isText()) {
      if (got && k!=kignore) {
	text->setText("");
        textl->setText("Text");
        kignore = -1;
	break;
      } else {
	text->setText(obj.isHole()
                      ? obj.asHole().ref
                      : obj.isPad()
                      ? obj.asPad().ref
                      : obj.isGroup()
                      ? obj.asGroup().ref
                      : obj.asText().text);
        textl->setText(obj.isHole()
                       ? "Pin"
                       : obj.isPad()
                       ? "Pin"
                       : obj.isGroup()
                       ? "Ref."
                       : "Text");
        if (text->text() != "")
          editor->properties().text = text->text();
        if (obj.isText())
          kignore = obj.asText().groupAffiliation();
        else if (obj.isGroup())
          kignore = obj.asGroup().refTextId();
	got = true;
      }
    }
  }
  if (kignore>0)
    textl->setText("Ref.");
}

void PBData::doFillArcAngle(int arcangle) {
  arc360->setChecked(arcangle==360);
  arc300->setChecked(arcangle==300);  
  arc240->setChecked(arcangle==240);  
  arc180->setChecked(arcangle==180);  
  arc120->setChecked(arcangle==120);  
  arc_90->setChecked(arcangle==90);
}

void PBData::fillArcAngle(QSet<int> const &objects, Group const &here) {
  //qDebug() << "fillarcangle" << objects;
  bool got = false;
  int arcangle = 0; // "invalid"
  // set angle if all are same
  for (int k: objects) {
    Object const &obj(here.object(k));
    if (obj.isArc()) {
      if (got) {
	if (obj.asArc().angle != arcangle) {
	  arcangle = 0;
	  break;
	}
      } else {
	arcangle = obj.asArc().angle;
	got = true;
      }
    }
  }
  if (arcangle!=0)
    editor->properties().arcangle = arcangle;
  if (got)
    doFillArcAngle(arcangle);
  else
    doFillArcAngle(editor->properties().arcangle);
  //if (arcangle==90)
  //  editor->properties().arcangle = -arcangle;
  
}

void PBData::fillLayer(QSet<int> const &objects, Group const &here) {
  Layer l = Layer::Invalid;
  bool got = false;
  // set layer if all are same
  for (int k: objects) {
    Object const &obj(here.object(k));
    switch (obj.type()) {
    case Object::Type::Pad:
      if (got) {
	if (obj.asPad().layer != l) {
	  l = Layer::Invalid;
	  break;
	}
      } else {
	l = obj.asPad().layer;
	got = true;
      }
      break;
    case Object::Type::Trace:
      if (got) {
	if (obj.asTrace().layer != l) {
	  l = Layer::Invalid;
	  break;
	}
      } else {
	l = obj.asTrace().layer;
	got = true;
      }
      break;
    case Object::Type::Text:
      if (got) {
	if (obj.asText().layer != l) {
	  l = Layer::Invalid;
	  break;
	}
      } else {
	l = obj.asText().layer;
	got = true;
      }
      break;
    case Object::Type::Arc:
      if (got) {
	if (obj.asArc().layer != l) {
	  l = Layer::Invalid;
	  break;
	}
      } else {
	l = obj.asArc().layer;
	got = true;
      }
      break;
    case Object::Type::Plane:
      if (got) {
	if (obj.asPlane().layer != l) {
	  l = Layer::Invalid;
	  break;
	}
      } else {
	l = obj.asPlane().layer;
	got = true;
      }
      break;
    default:
      break;
    }
  }
  if (got)
    checkLayer(l);
  if (l!=Layer::Invalid)
    editor->properties().layer = l;
}

void PBData::fillFontSize(QSet<int> const &objects, Group const &here) {
  bool got = false;
  // set fs if we have text, and all are same
  fs->setNoValue();
  for (int k: objects) {
    Object const &obj(here.object(k));
    if (obj.isText()) {
      if (got) {
	if (obj.asText().fontsize != fs->value())
	  fs->setNoValue();
	break;
      } else  {
	fs->setValue(obj.asText().fontsize);
	got = true;
      }
    }
  }
  if (fs->hasValue())
    editor->properties().fs = fs->value();  
}

void PBData::fillGroupProps(QSet<int> const &/*objects*/, Group const &here) {
  //qDebug() << "fillgroupprops" << here.nominalRotation();
  pkg->setText(here.attributes.value(Group::Attribute::Footprint));
  rot->setValue(here.nominalRotation());
  //  partno->setText(here.partno);
  notes->document()->setPlainText(here.attributes.value(Group::Attribute::Notes));
}

void PBData::updateAlignment() {
  x0 = xalign->extractDimension(r0, p0);
  y0 = yalign->extractDimension(r0, p0);
  x->setValue(x0 - ori.x);
  y->setValue(y0 - ori.y);
}  

void PBData::getPropertiesFromSelection() {
  QSet<int> objects(editor->selectedObjects());
  QSet<Point> points(editor->selectedPoints());
  Group const &here(editor->currentGroup());

  qDebug() << "getpropertiesfromselection" << objects.size() << points.size();
  getRectFromObjectsAndPoints(objects, here, points);
  getP0fromGroup(objects, here)
    || getP0fromPoints(points)
    || getP0fromText(objects, here)
    || getP0fromOrigin();
  updateAlignment();
  fillLinewidth(objects, here);
  fillWH(objects, here);
  fillDiamAndShape(objects,  here);
  fillRefText(objects, here);
  fillArcAngle(objects, here);  
  fillLayer(objects, here);  
  fillFontSize(objects, here);
  fillGroupProps(objects, here);
}

int PBData::arcAngle() const {
  if (arc360->isChecked())
    return 360;
 else if (arc300->isChecked())
   return 300;
 else if (arc240->isChecked())
   return 240;
 else if (arc180->isChecked())
   return 180;
 else if (arc120->isChecked())
   return 120;
 else if (arc_90->isChecked())
   return -90;
 else
   return 0; // invalid;
}

Layer PBData::checkedLayer() const {
  if (panel->isChecked())
    return Layer::Panel;
  else if (silk->isChecked())
    return Layer::Silk;
  else if (bsilk->isChecked())
    return Layer::BSilk;
  else if (top->isChecked())
    return Layer::Top;
  else if (bottom->isChecked())
    return Layer::Bottom;
  else
    return Layer::Invalid;
}

void PBData::hideAndShow() {
  xya->setEnabled(false);
  dima->setEnabled(false);
  texta->setEnabled(false);
  text->setEnabled(false);
  fs->setEnabled(false);
  arca->setEnabled(false);
  layera->setEnabled(false);
  orienta->setEnabled(false);

  linewidthc->setEnabled(false);
  wc->setEnabled(false);
  hc->setEnabled(false);
  idc->setEnabled(false);
  odc->setEnabled(false);
  slotlengthc->setEnabled(false);
  squarec->setEnabled(false);
  orientc->setVisible(true);
  flipped->setVisible(true);
  rotatec->setVisible(false);

  //qDebug() << "top acts" << xya << dima << arca;
  //qDebug() << "group" << xyg << dimg << arcg;
  //qDebug() << "conts" << squarec << arcc;

  switch (mode) {
  case Mode::Invalid: case Mode::BoardOutline:
    break;
  case Mode::Edit:
    hsEdit();
    break;
  case Mode::PlaceTrace:
    dima->setEnabled(true);
    linewidthc->setEnabled(true);
    layera->setEnabled(true);
    break;
  case Mode::PlaceHole:
    dima->setEnabled(true);
    idc->setEnabled(true);
    odc->setEnabled(true);
    slotlengthc->setEnabled(true);
    squarec->setEnabled(true);
    via->setEnabled(true);
    texta->setEnabled(true);
    text->setEnabled(true);
    textl->setText("Pin");
    break;
  case Mode::PlaceNPHole:
    dima->setEnabled(true);
    idc->setEnabled(true);
    odc->setEnabled(false);
    slotlengthc->setEnabled(true);
    squarec->setEnabled(false);
    texta->setEnabled(false);
    break;
  case Mode::PlacePad:
    dima->setEnabled(true);
    wc->setEnabled(true);
    hc->setEnabled(true);
    layera->setEnabled(true);
    texta->setEnabled(true);
    text->setEnabled(true);
    textl->setText("Pin");
    if (!anyLayerChecked()) {
      top->setChecked(true);
      editor->properties().layer = Layer::Top;
    }
    break;
  case Mode::PlaceText:
    texta->setEnabled(true);
    text->setEnabled(false); // not used any more. Popup is more convenient
    fs->setEnabled(true);
    textl->setText("Text");
    layera->setEnabled(true);
    orienta->setEnabled(true);
    flipped->setEnabled(true);
    if (!anyDirectionChecked()) {
      up->setChecked(true);
      editor->properties().rota = FreeRotation();
    }
    if (!anyLayerChecked()) {
      silk->setChecked(true);
      editor->properties().layer = Layer::Silk;
    }
    if (!fs->hasValue()) {
      Dim fs1 = Dim::fromInch(.050);
      fs->setValue(fs1);
      editor->properties().fs = fs1;
    }
    break;
  case Mode::PlaceArc:
    dima->setEnabled(true);
    linewidthc->setEnabled(true);
    idc->setEnabled(true);
    arca->setEnabled(true);
    orienta->setEnabled(true);
    flipped->setEnabled(false);
    if (!anyDirectionChecked()) {
      up->setChecked(true);
      editor->properties().rota = FreeRotation(0);
    }
    layera->setEnabled(true);
    if (!anyLayerChecked()) {
      silk->setChecked(true);
      editor->properties().layer = Layer::Silk;
    }
    break;
  case Mode::PlacePlane:
    layera->setEnabled(true);
    break;
  case Mode::PickupTrace:
    break;
  case Mode::PNPOrient:
    break;
  }

  //qDebug() << "crumbs" << (editor->breadcrumbs().size()>0);
  grouppropa->setVisible(editor->breadcrumbs().size()>0);
  //qDebug() << "vis" << grouppropg->isVisible() << partno->isVisible();
  
}

void PBData::hsEdit() {
  orienta->setEnabled(true);
  orientc->setVisible(false);
  flipped->setVisible(false);
  rotatec->setVisible(true);

  getPropertiesFromSelection();
  
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

  // Show id, od, sq., slot length if we have at least one hole
  for (int k: objects) {
    if (here.object(k).isHole()) {
      dima->setEnabled(true);
      idc->setEnabled(true);
      odc->setEnabled(true);
      slotlengthc->setEnabled(true);
      squarec->setEnabled(true);
      break;
    }
  }

  // Show id, slot length if we have at least one nphole
  for (int k: objects) {
    if (here.object(k).isNPHole()) {
      dima->setEnabled(true);
      idc->setEnabled(true);
      slotlengthc->setEnabled(true);
      break;
    }
  }

  // Show linewidth, id, arc if we have at least one arc
  for (int k: objects) {
    if (here.object(k).isArc()) {
      dima->setEnabled(true);
      linewidthc->setEnabled(true);
      idc->setEnabled(true);
      arca->setEnabled(true);
      break;
    }
  }
  
  // Show text if we have exactly one hole/pad or exactly one group or text
  if (objects.size()==1) {
    for (int k: objects) { // loop of 1, but each to write this way
      Object const &obj(here.object(k));
      if (obj.isGroup()
	  || obj.isHole()
	  || obj.isPad()
          || obj.isText()) {
	texta->setEnabled(true);
        text->setEnabled(true);
        if (obj.isGroup())
          fs->setEnabled(true);
      }
    }
  } else if (objects.size()==2) {
    for (int k: objects) { 
      Object const &obj(here.object(k));
      if (obj.isGroup()) {
	int tid = obj.asGroup().refTextId();
	if (objects.contains(tid)) {
	  // got group and its ref text
	  texta->setEnabled(true);
	  text->setEnabled(true);
          fs->setEnabled(true);
	}
      }
    }
  }

  // Show font size if we have at least one text
  for (int k: objects) {
    if (here.object(k).isText()) {
      texta->setEnabled(true);
      fs->setEnabled(true);
      break;
    }
  }

  // Show layer if we have at least one trace, pad, text, arc, or plane
  for (int k: objects) {
    Object const &obj(here.object(k));
    if (obj.isText() || obj.isPad() || obj.isTrace() || obj.isArc() || obj.isPlane()) {
      layera->setEnabled(true);
      break;
    }
  }
}

void PBData::setupUI() {
  parent->layout()->setContentsMargins(4,4,4,4);
  parent->setStyleSheet("QToolButton:checked { background-color: #ffffff; }"
                        "QLineEdit { padding: 0px 2px 0px 2px; }");
  auto makeGroup = [this](QAction **a) {
    Q_ASSERT(parent);
    Q_ASSERT(a);
    QWidget *group = new QWidget;
    auto *lay = new QVBoxLayout;
    lay->setSpacing(4);
    lay->setContentsMargins(0, 0, 0, 8);
    group->setLayout(lay);
    *a = parent->addWidget(group);
    return group;
  };
  
  auto makeContainer = [](QWidget *group) {
    Q_ASSERT(group);
    Q_ASSERT(group->layout());
    QWidget *container = new QWidget(group);
    auto *lay = new QHBoxLayout;
    lay->setSpacing(2);
    lay->setContentsMargins(0, 0, 0, 0);
    container->setLayout(lay);
    group->layout()->addWidget(container);
    return container;
  };

  auto makeSubContainer = makeContainer;

  auto makeDimSpinner = [](QWidget *container,
			   Dim step=Dim::fromInch(.005)) {
    Q_ASSERT(container);
    Q_ASSERT(container->layout());
    DimSpinner *s = new DimSpinner(container);
    s->setStep(step);
    container->layout()->addWidget(s);
    return s;
  };

  auto makeRotSpinner = [](QWidget *container) {
    Q_ASSERT(container);
    Q_ASSERT(container->layout());
    QSpinBox *s = new QSpinBox(container);
    s->setRange(0, 360);
    s->setSuffix("°");
    s->setSingleStep(90);
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

  auto makeTEdit = [](QWidget *container) {
    Q_ASSERT(container);
    Q_ASSERT(container->layout());
    auto *s = new QTextEdit;
    container->layout()->addWidget(s);
    return s;
  };

  auto makeLabel = [](QWidget *container, QString txt, QString tip="") {
    Q_ASSERT(container);
    Q_ASSERT(container->layout());
    QLabel *s = new QLabel(txt);
    if (!tip.isEmpty())
      s->setToolTip(tip);
    container->layout()->addWidget(s);
    return s;
  };

  auto makeIcon = [](QWidget *container, QString icon, QString tip="") {
    Q_ASSERT(container);
    Q_ASSERT(container->layout());
    QLabel *s = new QLabel;
    s->setPixmap(QIcon(":icons/" + icon + ".svg").pixmap(16));
    s->setToolTip(tip.isEmpty() ? icon : tip);
    container->layout()->addWidget(s);
    return s;
  };

  auto addLine = [this](QWidget *container) {
    auto *line = new QFrame(container);
    line->resize(3, 100);
    line->setFrameShape(QFrame::VLine);
    line->setFrameShadow(QFrame::Sunken);
    container->layout()->addWidget(line);
  };

  auto makeIconTool = [this](QWidget *container, QString icon,
			     bool chkb=false, bool ae=false, QString tip="",
			     QKeySequence seq=QKeySequence()) {
    Q_ASSERT(container);
    Q_ASSERT(container->layout());
    QToolButton *s = new QToolButton;
    QAction *a = new QAction(QIcon(":icons/" + icon + ".svg"), icon);
    s->setDefaultAction(a);
    a->setCheckable(chkb);
    if (ae) {
      for (QToolButton *oth: exclusiveGroups[container]) {
	QObject::connect(oth, &QToolButton::clicked,
			 [s, oth]() {
			   //qDebug() << "click1" << s << oth;
			   s->setChecked(false);
			   oth->setChecked(true);
			 });
	QObject::connect(s,  &QToolButton::clicked,
			 [s, oth]() {
			   //qDebug() << "click2" << s << oth;
			   s->setChecked(true);
			   oth->setChecked(false);
			 });
      }
      exclusiveGroups[container].insert(s);
    }
    container->layout()->addWidget(s);
    if (tip.isEmpty())
      tip = icon;
    if (!seq.isEmpty()) {
      a->setShortcut(seq);
      tip += " (" + seq.toString() + ")";
    }
    a->setToolTip(tip);
    parent->parentWidget()->addAction(a);
    return a;
  };


  grouppropg = makeGroup(&grouppropa);
  auto *cnt = makeContainer(grouppropg);
  makeLabel(cnt, "Pkg.", "Footprint");
  pkg = makeEdit(cnt);
  QObject::connect(pkg, &QLineEdit::textEdited,
		   editor, [this](QString txt) {
                     editor->setCurrentGroupAttribute(Group::Attribute::Footprint, txt);
                   });
  QObject::connect(pkg, &QLineEdit::returnPressed,
		   editor,[this]() {
                     editor->setCurrentGroupAttribute(Group::Attribute::Footprint, pkg->text());
                   });

  cnt = makeContainer(grouppropg);
  makeLabel(cnt, "Nominal rotation", "Nominal rotation (deg. CCW; used only for PnP export)");
  rot = makeRotSpinner(cnt);
  QObject::connect(rot, QOverload<int>::of(&QSpinBox::valueChanged),
		   editor, [this](int val) {
                     editor->setCurrentGroupRotation(val);
                   });
  
  // cnt = makeContainer(grouppropg);
  // makeLabel(cnt, "Part", "Part number");
  //  partno = makeEdit(cnt);
  // QObject::connect(partno, &QLineEdit::textEdited,
  //                  editor, [this](QString txt) {
  //                    editor->setCurrentGroupPartno(txt);
  //                  });
  // QObject::connect(partno, &QLineEdit::returnPressed,
  //       	   editor, [this]() {
  //                    editor->setCurrentGroupPartno(partno->text());
  //                  });
  cnt = makeContainer(grouppropg);
  makeLabel(cnt, "Notes", "Part notes");
  notes = makeTEdit(cnt);
  QFrame* line = new QFrame();
  line->setFrameShape(QFrame::HLine);
  line->setFrameShadow(QFrame::Sunken);
  grouppropg->layout()->addWidget(line);
  QObject::connect(notes, &QTextEdit::textChanged,
		   editor, [this]() {
                     editor->setCurrentGroupAttribute(Group::Attribute::Notes,
                                           notes->document()->toPlainText());
                   });
  //  qDebug() << "premg" << xya;
  xyg = makeGroup(&xya);
  //qDebug() << "postmg" << xya;
  xc = makeContainer(xyg);
  auto *xc1 = makeSubContainer(xc);
  makeLabel(xc1, "X", "Distance from left");
  xabsinc = new AbsIncToggle();
  QObject::connect(xabsinc, &AbsIncToggle::clicked,
                   parent, [this]() { clickAbsInc(Qt::Horizontal); });
  xc1->layout()->addWidget(xabsinc);
  xalign = new AlignToggle(Qt::Horizontal);
  QObject::connect(xalign, &AlignToggle::alignmentChanged,
                   parent, [this]() { updateAlignment(); });
  xc1->layout()->addWidget(xalign);
  x = makeDimSpinner(xc, Dim::fromInch(.050));
  QObject::connect(x, &DimSpinner::valueEdited,
		   [this](Dim d) {
		     d += ori.x;
                     Point dp(d - x0, Dim());
                     r0.translate(dp);
                     p0 += dp;
		     editor->translate(dp);
		     x0 = d; });
  yc = makeContainer(xyg);
  auto *yc1 = makeSubContainer(yc);
  makeLabel(yc1, "Y", "Distance from top");
  yabsinc = new AbsIncToggle();
  QObject::connect(yabsinc, &AbsIncToggle::clicked,
                   [this]() { clickAbsInc(Qt::Vertical); });
  yc1->layout()->addWidget(yabsinc);
  yalign = new AlignToggle(Qt::Vertical);
  QObject::connect(yalign, &AlignToggle::alignmentChanged,
                   parent, [this]() { updateAlignment(); });  
  yc1->layout()->addWidget(yalign);
  y = makeDimSpinner(yc, Dim::fromInch(.050));
  QObject::connect(y, &DimSpinner::valueEdited,
		   [this](Dim d) {
		     d += ori.y;
                     Point dp(Dim(), d - y0);
                     r0.translate(dp);
                     p0 += dp;
		     editor->translate(dp);                     
		     y0 = d; });


  dimg = makeGroup(&dima);

  linewidthc = makeContainer(dimg);
  makeIcon(linewidthc, "Width", "Line width");
  linewidth = makeDimSpinner(linewidthc);
  linewidth->setValue(Dim::fromInch(.010));
  QObject::connect(linewidth, &DimSpinner::valueEdited,
		   [this](Dim d) { editor->setLineWidth(d); });

  ((QVBoxLayout*)(dimg->layout()))->addSpacing(8);
  
  idc = makeContainer(dimg);
  //makeLabel(idc, "⌀", "Hole diameter");
  makeIcon(idc, "Diameter", "Hole diameter");
  id = makeDimSpinner(idc);
  id->setMinimumValue(Dim::fromMM(0.3));
  id->setValue(Dim::fromInch(.040));
  QObject::connect(id, &DimSpinner::valueEdited,
		   [this](Dim d) {
		     if (od->hasValue()
			 && (od->value() < d + minRingWidth))
		       od->setValue(d + minRingWidth, true);
		     editor->setID(d); });

  slotlengthc = makeContainer(dimg);
  makeIcon(slotlengthc, "SlotLength", "Slot length");
  slotlength = makeDimSpinner(slotlengthc);
  slotlength->setMinimumValue(Dim::fromInch(0.00));
  QObject::connect(slotlength, &DimSpinner::valueEdited,
		   [this](Dim d) {
		     editor->setSlotLength(d);
                   });
  
  odc = makeContainer(dimg);
  makeLabel(odc, "OD", "Pad diameter");
  od = makeDimSpinner(odc);
  od->setMinimumValue(Dim::fromInch(0.020));
  od->setValue(Dim::fromInch(.065));
  QObject::connect(od, &DimSpinner::valueEdited,
		   [this](Dim d) {
		     if (id->hasValue()
			 && (id->value() > d - minRingWidth))
		       id->setValue(d - minRingWidth, true);
		     editor->setOD(d);
		   });

  //  auto *holec = makeContainer(dimg);
  squarec = makeContainer(dimg);
  makeLabel(squarec, "Shape");
  circle = makeIconTool(squarec, "Round", true, true, "Round");
  circle->setChecked(true);
  square = makeIconTool(squarec, "Square", true, true, "Square");
  addLine(squarec);
  via = makeIconTool(squarec, "Via", true, false, "Via");

  QObject::connect(square, &QAction::triggered,
		   [this]() {
		     editor->setSquare(true);
		   });
  QObject::connect(circle, &QAction::triggered,
		   [this]() {
		     editor->setSquare(false);
		   });
  QObject::connect(via, &QAction::triggered,
		   [this](bool chk) {
		     editor->setVia(chk);
		   });

  ((QVBoxLayout*)(dimg->layout()))->addSpacing(8);

  wc = makeContainer(dimg);
  makeLabel(wc, "Width", "Pad width");
  w = makeDimSpinner(wc);
  w->setValue(Dim::fromInch(.020));
  w->setMinimumValue(Dim::fromInch(0.005));
  QObject::connect(w, &DimSpinner::valueEdited,
		   [this](Dim d) {
		     editor->setWidth(d);
                   });

  hc = makeContainer(dimg);
  makeLabel(hc, "Height", "Pad height");
  h = makeDimSpinner(hc);
  h->setValue(Dim::fromInch(.040));
  h->setMinimumValue(Dim::fromInch(0.005));
  QObject::connect(h, &DimSpinner::valueEdited,
		   [this](Dim d) {
		     editor->setHeight(d);
                   });

  textg = makeGroup(&texta);

  auto *c5 = makeContainer(textg);
  textl = makeLabel(c5, "Text", "Text or object reference");
  text = makeEdit(c5);
  QObject::connect(text, &QLineEdit::textEdited,
		   [this](QString txt) { editor->setRefText(txt); });
  QObject::connect(text, &QLineEdit::returnPressed,
		   [this]() { editor->setRefText(text->text()); });

  auto *c1 = makeContainer(textg);
  makeLabel(c1, "Font size", "Font size");
  fs = makeDimSpinner(c1);
  fs->setValue(Dim::fromInch(.050));
  QObject::connect(fs, &DimSpinner::valueEdited,
		   [this](Dim d) { editor->setFontSize(d); });

  arcg = makeGroup(&arca);

  arcc = makeContainer(arcg);
  arc360 = makeIconTool(arcc, "Arc360a", true, true, "Full circle");
  arc360->setChecked(true);
  arc300 = makeIconTool(arcc, "Arc300a", true, true, "300° arc");
  arc240 = makeIconTool(arcc, "Arc240a", true, true, "240° arc");
  arc180 = makeIconTool(arcc, "Arc180a", true, true, "Half circle");
  arc120 = makeIconTool(arcc, "Arc120a", true, true, "120° arc");
  arc_90 = makeIconTool(arcc, "Arc-90a", true, true, "Quarter circle");
  
  QObject::connect(arc360, &QAction::triggered,
		   [this]() { editor->setArcAngle(360); });
  QObject::connect(arc300, &QAction::triggered,
		   [this]() { editor->setArcAngle(300); });
  QObject::connect(arc240, &QAction::triggered,
		   [this]() { editor->setArcAngle(240); });
  QObject::connect(arc180, &QAction::triggered,
		   [this]() { editor->setArcAngle(180); });
  QObject::connect(arc120, &QAction::triggered,
		   [this]() { editor->setArcAngle(120); });
  QObject::connect(arc_90, &QAction::triggered,
		   [this]() { editor->setArcAngle(-90); });


  QWidget *x = new QWidget;
  x->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Expanding);
  parent->addWidget(x);
  
  orientg = makeGroup(&orienta);

  auto *c3 = makeContainer(orientg);
  orientc = makeContainer(c3);
  rotatec = makeContainer(c3);
  up = makeIconTool(orientc, "OrientUp", true, true);
  QObject::connect(up, &QAction::triggered,
		   [this](bool b) {
		     if (b) 
		       editor->setRotation(0);
		   });
  right = makeIconTool(orientc, "OrientRight", true, true);
  QObject::connect(right, &QAction::triggered,
		   [this](bool b) {
		     if (b) 
		       editor->setRotation(90);
		   });
  down = makeIconTool(orientc, "OrientDown", true, true);
  QObject::connect(down, &QAction::triggered,
		   [this](bool b) {
		     if (b) 
		       editor->setRotation(180);
		   });
  left = makeIconTool(orientc, "OrientLeft", true, true);
  QObject::connect(left, &QAction::triggered,
		   [this](bool b) {
		     if (b)
		       editor->setRotation(270);
		   });
  flipped = makeIconTool(orientc, "Flipped", true);
  QObject::connect(flipped, &QAction::triggered,
		   [this](bool b) {
		     editor->setFlipped(b);
		   });

  ccw = makeIconTool(rotatec, "RotateCCW", false, false, "Rotate left");
  QObject::connect(ccw, &QAction::triggered,
		   [this]() { editor->rotateCCW(); });
  cw = makeIconTool(rotatec, "RotateCW", false, false, "Rotate right");
  QObject::connect(cw, &QAction::triggered,
		   [this]() { editor->rotateCW(); });
  fliph = makeIconTool(rotatec, "FlipH", false, false, "Flip left to right");
  QObject::connect(fliph, &QAction::triggered,
		   [this]() { editor->flipH(); });
  flipv = makeIconTool(rotatec, "FlipV", false, false, "Flip top to bottom");
  QObject::connect(flipv, &QAction::triggered,
		   [this]() { editor->flipV(); });

  layerg = makeGroup(&layera);

  auto *lc = makeContainer(layerg);
  makeLabel(lc, "Layer");

  silk = makeIconTool(lc, "LayerSilk", true, false, "",
		      QKeySequence(Qt::Key_1));
  QObject::connect(silk, &QAction::triggered,
		   [this]() {
                     checkLayer(Layer::Silk);
		     editor->setLayer(Layer::Silk);
		     });

  top = makeIconTool(lc, "LayerTop", true, false, "",
		     QKeySequence(Qt::Key_2));
  QObject::connect(top, &QAction::triggered,
		   [this]() {
		     checkLayer(Layer::Top);
		     editor->setLayer(Layer::Top);
		     });

  bottom = makeIconTool(lc, "LayerBottom", true, false, "",
			QKeySequence(Qt::Key_3));
  QObject::connect(bottom, &QAction::triggered,
		   [this]() {
                     checkLayer(Layer::Bottom);
		     editor->setLayer(Layer::Bottom);
		   });

  bsilk = makeIconTool(lc, "LayerBSilk", true, false, "Bottom silk",
		      QKeySequence(Qt::Key_4));
  QObject::connect(bsilk, &QAction::triggered,
		   [this]() {
                     checkLayer(Layer::BSilk);
		     editor->setLayer(Layer::BSilk);
		     });

  panel = makeIconTool(lc, "LayerPanel", true, false, "",
		      QKeySequence(Qt::Key_5));
  QObject::connect(panel, &QAction::triggered,
		   [this]() {
		     checkLayer(Layer::Panel);
		     editor->setLayer(Layer::Panel);
		     });

  top->setChecked(true);
  //qDebug() << "post" << xya;


  
}  

Propertiesbar::Propertiesbar(Editor *editor, QWidget *parent): QToolBar(parent) {
  setFloatable(false);
  setMovable(false);
  d = new PBData;
  d->parent = this;
  d->editor = editor;
  d->metric = false;
  d->mode = Mode::Edit;
  d->setupUI();
}

void Propertiesbar::reflectMode(Mode m) {
  d->mode = m;
  d->editor->properties().text = "";
  d->text->setText("");
  if (m==Mode::PlaceHole) {
    bool sq = d->square->isChecked();
    d->square->setChecked(sq);
    d->circle->setChecked(!sq);
    d->editor->setSquare(sq);
    d->editor->setVia(d->via->isChecked());
    if (d->od->value() < d->id->value() + minRingWidth)
      d->od->setValue(d->id->value() + minRingWidth);
  }
  if (m==Mode::PlaceArc || m==Mode::PlaceText) {
    if (!d->anyLayerChecked()) {
      d->silk->setChecked(true);
      d->editor->properties().layer = Layer::Silk;
    }
  }
  if (m==Mode::PlaceTrace || m==Mode::PlacePad) {
    if (!d->anyLayerChecked()) {
      //qDebug() << "setting layer top";
      d->top->setChecked(true);
      d->editor->properties().layer = Layer::Top;
    }
  }
  if (m==Mode::PlaceArc) {
    if (d->arcAngle()==0) {
      d->editor->properties().arcangle = 360;
      d->doFillArcAngle(360);
    }
  }
  if (m==Mode::PlacePlane) {
    if (!layerIsCopper(d->checkedLayer())) {
      //qDebug() << "PlacePlane - selecting bottom layer";
      d->checkLayer(Layer::Bottom);
      d->editor->properties().layer = Layer::Bottom;
    }
  }
  d->hideAndShow();
}

void Propertiesbar::reflectSelection() {
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
  d->slotlength->setMetric(m);
  d->fs->setMetric(m);
  updateGeometry();
}

void Propertiesbar::forwardAllProperties() {
  if (!d->editor)
    return;
  d->editor->setLineWidth(d->linewidth->value());
  d->editor->setLayer(d->checkedLayer());
  d->editor->setID(d->id->value());
  d->editor->setOD(d->od->value());
  d->editor->setSquare(d->square->isChecked());
  d->editor->setFontSize(d->fs->value());
  d->editor->setRefText(d->text->text());
  d->editor->setRotation(d->right->isChecked() ? 90
			 : d->down->isChecked() ? 180
			 : d->left->isChecked() ? 270
			 : 0);
  d->editor->setFlipped(d->flipped->isChecked());
}

void Propertiesbar::stepPinNumber() {
  int pin = d->text->text().toInt();
  if (pin>0) {
    QString txt = QString::number(pin+1);
    d->text->setText(txt);
    d->editor->properties().text = txt;
  } else {
    d->text->setText("");
    d->editor->properties().text = "";
  }
}

void PBData::clickAbsInc(Qt::Orientation o) {
  if (o == Qt::Horizontal) {
    if (x0 == ori.x) {
      xabsinc->setAbs();
      parent->setUserOrigin(Point(Dim(), ori.y));
    } else {
      xabsinc->setInc();
      parent->setUserOrigin(Point(x0, ori.y));
    }
  } else { // vertical
    if (y0 == ori.y) {
      yabsinc->setAbs();
      parent->setUserOrigin(Point(ori.x, Dim()));
    } else {
      yabsinc->setInc();
      parent->setUserOrigin(Point(ori.x, y0));
    }
  }

}

void Propertiesbar::toggleAbsInc() {
  QSet<int> objects(d->editor->selectedObjects());
  QSet<Point> points(d->editor->selectedPoints());
  if ((objects.isEmpty() && points.isEmpty())
      || (d->x0 == d->ori.x && d->y0 == d->ori.y)) {
    d->xabsinc->setAbs();
    d->yabsinc->setAbs();
    setUserOrigin(Point());
  } else {
    d->xabsinc->setInc();
    d->yabsinc->setInc();
    setUserOrigin(Point(d->x0, d->y0));
  }
}
