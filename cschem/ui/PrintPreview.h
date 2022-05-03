// PrintPreview.h

#ifndef PRINTPREVIEW_H

#define PRINTPREVIEW_H

#include <QWidget>

class PrintPreview: public QObject {
  Q_OBJECT
public:
  PrintPreview(QWidget *parent, class Scene *scene, QString filename);
  virtual ~PrintPreview();
  static void preview(QWidget *parent, class Scene *scene, QString filename);
  static void print(QWidget *parent, class Scene *scene, QString filename);
public slots:
  void paintRequest(class QPrinter *);
private:
  class Scene *scene;
  QString filename;
};

#endif
