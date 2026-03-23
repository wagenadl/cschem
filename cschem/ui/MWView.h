// MWView.h

#ifndef MWVIEW_H

#include <QGraphicsView>
#include <QWheelEvent>

#define MWVIEW_H

class MWView: public QGraphicsView {
  Q_OBJECT;
signals:
  void rescaled();
public:
  MWView(QWidget *parent=0): QGraphicsView(parent) {
    /* Use either AnchorUnderMouse or AnchorViewCenter to set anchor for
       zooming with [control]+[+/-].
       Zooming with [control]+[scroll wheel]
       is always relative to mouse position. */
    setTransformationAnchor(QGraphicsView::NoAnchor);
    setBackgroundBrush(Qt::white);
  }
  virtual ~MWView() { }
  virtual void wheelEvent(QWheelEvent *e) override {
    auto anchor = transformationAnchor();
    if (e->modifiers() & Qt::ControlModifier) {
      double scl = std::pow(2, e->angleDelta().y() / 240.0);
      setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
      scale(scl, scl);
      emit rescaled();
    } else {
      setTransformationAnchor(QGraphicsView::NoAnchor);
      if (e->deviceType()==QInputDevice::DeviceType::TouchPad) {
        QPoint delta = e->pixelDelta() * 2;
        translate(delta.x(), delta.y());
      } else { // scroll wheel
        int delta = e->angleDelta().y();
        if (e->modifiers() & Qt::ShiftModifier) 
          translate(delta, 0);  
        else
          translate(0, delta);
      }
    }
    setTransformationAnchor(anchor); 
  }
  virtual void keyPressEvent(QKeyEvent *e) override {
    QGraphicsView::keyPressEvent(e);
    /*
    // This does not work, because bizarrely, press and release is
    // reported as press. a second press and release is reported as release.
    qDebug() << "KeyPress" << e->key() << int(Qt::Key_Alt);
    if (e->key()==Qt::Key_Alt)
        setDragMode(QGraphicsView::ScrollHandDrag);
    */
  }
  virtual void keyReleaseEvent(QKeyEvent *e) override {
    QGraphicsView::keyReleaseEvent(e);
    /*
    qDebug() << "KeyRelease" << e->key() << int(Qt::Key_Alt);
    if (e->key()==Qt::Key_Alt)
        setDragMode(QGraphicsView::NoDrag);
    */
  }
};

#endif
