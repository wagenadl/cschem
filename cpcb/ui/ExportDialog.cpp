// ExportDialog.cpp

#include "ExportDialog.h"
#include "ui_ExportDialog.h"
#include <QFileInfo>
#include <QFileDialog>
#include <QTemporaryDir>
#include <QProcess>
#include <QMessageBox>
#include "gerber/GerberWriter.h"
#include "gerber/PasteMaskWriter.h"
#include "gerber/FrontPanelWriter.h"
#include "data/PickNPlace.h"
#include "data/BOMTable.h"
#include "data/LinkedSchematic.h"
#include <QSettings>
#include "DimSpinner.h"

ExportDialog::ExportDialog(QWidget *parent): QDialog(parent) {
  ui = new Ui_ExportDialog;
  ui->setupUi(this);
  connect(ui->savepnp, &QCheckBox::toggled,
          this, [this](bool x) {
            ui->saveunplaced->setEnabled(x);
            ui->saveimage->setEnabled(x);
          });
  QSettings stg;
  Dim dflt = Dim::fromString(stg.value("shrinkage",
                                       Dim::fromInch(0.005).toString())
                             .toString());
  ui->shrinkage->setValue(dflt);
}

ExportDialog::~ExportDialog() {
  delete ui;
}

bool ExportDialog::runDialog(QString pcbfilename, QString exportdir) {
  pwd = exportdir;
  QFileInfo fi(pcbfilename);
  if (pcbfilename.isEmpty())
    ui->gerberfilename->setText("");
  else
    ui->gerberfilename->setText(fi.absolutePath()
                                + "/" + fi.completeBaseName() + ".zip");
  return exec();
}

QString ExportDialog::filename() const {
  return ui->gerberfilename->text();
}

void ExportDialog::browse() {
  QString fn = ui->gerberfilename->text();
  QString sug = fn.isEmpty() ? pwd : fn;
  fn = QFileDialog::getSaveFileName(this, "Gerber filename…",
                                    sug,
                                    "Gerber files (*.zip");
  if (!fn.isEmpty())
    ui->gerberfilename->setText(fn);
}



void ExportDialog::gerbernamechange() {
  QString fn = ui->gerberfilename->text();
  QFileInfo fi(fn);
  if (fn.isEmpty()) {
    ui->bomfilename->setText("");
    ui->pnpfilename->setText("");
    ui->unplacedfilename->setText("");
    ui->imagefilename->setText("");
  } else {
    QString base = fi.completeBaseName();
    ui->bomfilename->setText(".../" + base + "-bom.csv");
    ui->pnpfilename->setText(".../" + base + "-pnp.csv");
    ui->unplacedfilename->setText(".../" + base + "unplaced.csv");
    ui->imagefilename->setText(".../" + base + "-pnpimage.png");
  }
}

bool ExportDialog::saveAccordingly(Layout const &pcblayout,
                                   LinkedSchematic const &schematic) {
  QString gerberfn = ui->gerberfilename->text();
  if (gerberfn.isEmpty())
    return false;
  if (!saveGerber(pcblayout))
    return false;

  BOMTable bom(pcblayout.root());
  bom.augment(schematic.circuit());
  if (ui->savebom->isChecked())
    if (!saveBOM(bom))
      return false;

  PickNPlace pnp(pcblayout.root(),
                 ui->smtonly->isChecked()
                 ? PickNPlace::Scope::SMTOnly
                 : PickNPlace::Scope::SMTAndThruHole);
  if (ui->savepnp->isChecked())
    if (!savePnP(pnp))
      return false;

  if (ui->saveunplaced->isChecked())
    if (!saveUnplaced(pnp, bom))
      return false;

  if (ui->savemask->isChecked())
    if (!savePasteMask(pcblayout))
      return false;

  if (ui->savefrontpanel->isChecked())
    if (!saveFrontPanel(pcblayout))
      return false;

  if (ui->saveimage->isChecked())
    if (!saveImage(pcblayout, pnp))
      return false;
  return true;
}

bool ExportDialog::saveGerber(class Layout const &pcblayout) {
  QFileInfo fi(ui->gerberfilename->text());
  QString fn = fi.absoluteFilePath();
  QString base = fi.completeBaseName();
  QTemporaryDir td;
  bool ok = false;
  qDebug() << "savegerber" << td.isValid() << base;
  if (td.isValid()) {
    if (GerberWriter::write(pcblayout, td.filePath(base))) {
      QDir cwd = QDir::current();
      QDir::setCurrent(td.path());
      QStringList args;
      args << "-r" << fn << base;
      QDir::root().remove(fn);
      ok = QProcess::execute("zip", args)==0;
      QDir::setCurrent(cwd.absolutePath());
    }
  }
  QDir(td.filePath(base)).removeRecursively();
  if (!ok)
    QMessageBox::warning(parentWidget(), "cpcb",
                         "Could not export Gerber as “"
                         + fn + "”",
                         QMessageBox::Ok);
  return ok;
}

QString ExportDialog::filename(QString ext) const {
  QFileInfo fi(ui->gerberfilename->text());
  QString fn = fi.absoluteFilePath();
  QString dir = fi.absolutePath();
  QString base = fi.completeBaseName();
  return  dir + "/" + base + ext;
}

bool ExportDialog::savePnP(PickNPlace const &pnp) {
  QString ofn = filename("-pnp.csv");

  if (pnp.saveCSV(ofn))
    return true;
  
  QMessageBox::warning(parentWidget(), "cpcb",
                       "Could not export PnP table as “"
                       + ofn + "”",
                       QMessageBox::Ok);
  return false;
}

bool ExportDialog::saveBOM(BOMTable const &bom) {
  QString ofn = filename("-bom.csv");
  if (bom.saveCSV(ofn, ui->compactbom->isChecked()))
    return true;
      
  QMessageBox::warning(parentWidget(), "cpcb",
                       "Could not export BOM as “"
                       + ofn + "”",
                       QMessageBox::Ok);
  return false;
}

bool ExportDialog::savePasteMask(Layout const &pcblayout) {
  QString ofn = filename("-mask.svg");
  Dim shrink = ui->shrinkage->value();
  QSettings stg;
  stg.setValue("shrinkage", shrink.toString());

  PasteMaskWriter pmw;
  pmw.setShrinkage(shrink);
  if (pmw.write(pcblayout, ofn))
    return true;
      
  QMessageBox::warning(parentWidget(), "cpcb",
                       "Could not export paste mask as “"
                       + ofn + "”",
                       QMessageBox::Ok);
  return false;
}

bool ExportDialog::saveFrontPanel(Layout const &pcblayout) {
  QString ofn = filename("-frontpanel.svg");

  FrontPanelWriter pmw;
  if (pmw.write(pcblayout, ofn))
    return true;
      
  QMessageBox::warning(parentWidget(), "cpcb",
                       "Could not export front panel as “"
                       + ofn + "”",
                       QMessageBox::Ok);
  return false;
}

bool ExportDialog::saveUnplaced(PickNPlace const &pnp, BOMTable const &bom) {  
  QString ofn = filename("-unplaced.txt");
  QFile funp(ofn);
  if (!funp.open(QFile::WriteOnly | QFile::Text)) {
    QMessageBox::warning(parentWidget(), "cpcb",
                         "Could not export unplaced PnP as “"
                         + ofn + "”",
                         QMessageBox::Ok);
    return false;
  }

  QStringList uni = pnp.unplacedRefs();
  QList<QStringList> rows = bom.toList(true, &uni);
  QTextStream ts(&funp);
  for (QStringList row: rows) {
    ts << row[1];
    if (row[0] != "")
      ts << " (" << row[0] << ")";
    ts << "\n";
  }
  return true;
}

bool ExportDialog::saveImage(Layout const &pcblayout, PickNPlace const &pnp) {
    qDebug() << "PNP Image NYI";
    return false;
}
