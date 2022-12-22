// PrintPreview.cpp

#include "PrintPreview.h"
#include "Scene.h"
#include <QPrintDialog>
#include <QPrintPreviewDialog>
#include <QPrinter>
#include <QPainter>
#include <QDebug>

PrintPreview::PrintPreview(QWidget *parent,
			   Scene *scene,
			   QString filename): QObject(parent),
					     scene(scene),
					     filename(filename) {
}

PrintPreview::~PrintPreview() {
}

void PrintPreview::paintRequest(QPrinter *prt) {
  QPainter p(prt);
  int destw = prt->width();
  int desth = prt->height();
  QRectF sourcerect = scene->itemsBoundingRect();
  double sourcew = sourcerect.width();
  double sourceh = sourcerect.height();
  double xr = destw/sourcew;
  double yr = desth/sourceh;
  double r = (xr<yr) ? xr : yr;
  if (r>10)
    r = 10;
  QRectF destrect(QPointF(destw/2 - r*sourcew/2, desth/2 - r*sourceh/2),
		  QSizeF(r*sourcew, r*sourceh));
  scene->render(&p, destrect, sourcerect);
}


void PrintPreview::preview(QWidget *parent, Scene *scene, QString filename) {
  PrintPreview pp(parent, scene, filename);
  QPrinter prt(QPrinter::HighResolution);
  prt.setDocName(filename);
  prt.setPageSize(QPageSize(QPageSize::Letter));
  QPrintPreviewDialog dlg(&prt, parent);
  connect(&dlg, &QPrintPreviewDialog::paintRequested,
	  &pp, &PrintPreview::paintRequest);
  dlg.exec();
}

void PrintPreview::print(QWidget *parent, Scene *scene, QString filename) {
  PrintPreview pp(parent, scene, filename);
  QPrinter prt(QPrinter::HighResolution);
  prt.setDocName(filename);
  prt.setPageSize(QPageSize(QPageSize::Letter));
  QPrintDialog dlg(&prt, parent);
  if (dlg.exec() == QDialog::Accepted) {
    pp.paintRequest(&prt);
  }
}
