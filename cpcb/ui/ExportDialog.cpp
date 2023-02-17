// ExportDialog.cpp

#include "ExportDialog.h"
#include "ui_ExportDialog.h"
#include <QFileInfo>
#include <QFileDialog>

ExportDialog::ExportDialog(QWidget *parent): QDialog(parent) {
  ui = new Ui_ExportDialog;
  ui->setupUi(this);
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
    QString base = fn.completeBaseName();
    ui->bomfilename->setText(".../" + base + "-bom.csv");
    ui->pnpfilename->setText(".../" + base + "-pnp.csv");
    ui->unplacedfilename->setText(".../" + base + "unplaced.csv");
    ui->imagefilename->setText(".../" + base + "-pnpimage.png");
  }
}

bool ExportDialog::saveAccordingly(class Layout const &pcblayout,
                                   class LinkedSchematic const &schematic) {
  QString gerberfn = ui->gerberfilename->getText();
  if (gerberfn.isEmpty())
    return false;
  if (!saveGerber(pcblayout))
    return false;
  if (!savePnP(pcblayout, schematic))
    return false;
}

bool ExportDialog::saveGerber(class Layout const &pcblayout) {
  QFileInfo fi(ui->gerberfilename->getText());
  QString fn = fi.absoluteFilePath();
  QString base = fi.completeBaseName();
  QTemporaryDir td;
  bool ok = false;
  if (td.isValid()) {
    if (GerberWriter::write(editor->pcbLayout(), td.filePath(base))) {
      QDir cwd = QDir::current();
      QDir::setCurrent(td.path());
      QStringList args;
      args << "-r" << fn << base;
      ok = QDir::root().remove(fn)
        && (QProcess::execute("zip", args)==0);
      QDir::setCurrent(cwd.absolutePath());
    }
  }
  QDir(td.filePath(base)).removeRecursively();
  if (!ok)
    QMessageBox::warning(parent(), "cpcb",
                         "Could not export pcb as “"
                         + fn + "”",
                         QMessageBox::Ok);
  return ok;
}

bool ExportDialog::savePNP(class Layout const &pcblayout,
                           class LinkedSchematic const &schematic) ) {
  QFileInfo fi(ui->gerberfilename->getText());
  QString fn = fi.absoluteFilePath();
  QString base = fi.completeBaseName();
  QString pnpfn = dir + "/" + base + "-pnp.csv";

  PickNPlace pnp(pcblayout.root(),
                 ui->smtonly->isChecked()
                 ? PickAndPlace::Scope::SMTOnly
                 : PickAndPlace::Scope::SMTAndThruHole);
  
  QString bomfn = dir + "/" + base + "-bom.csv";
  QString unplacedfn = dir + "/" + base + "-unplaced.csv";
  QString imagefn = dir + "/" + base + "-pngimage.png";

  
}
