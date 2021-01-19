#include "SettingsDialog.h"

#include <QMessageBox>

#include "Settings.h"
#include "ui_SettingsDialog.h"

SettingsDialog::SettingsDialog(QWidget *parent)
    : QDialog(parent), ui(new Ui::SettingsDialog) {
  ui->setupUi(this);
  ui->chordPreviewCheck->setChecked(Settings::ChordPreview::get());
  ui->customStyleCheck->setChecked(Settings::CustomStyle::get());
  ui->pauseReseekCheck->setChecked(Settings::SpacebarStop::get());
  ui->velocityDragCheck->setChecked(Settings::VelocityDrag::get());
  ui->swapScrollOrientationCheck->setChecked(
      Settings::SwapScrollOrientation::get());
  ui->swapZoomOrientationCheck->setChecked(
      Settings::SwapZoomOrientation::get());

  connect(ui->customStyleCheck, &QCheckBox::toggled, [this](bool) {
    QMessageBox::information(
        this, tr("Style change"),
        tr("Style change will take effect after program restart."));
  });
  connect(this, &QDialog::accepted, this, &SettingsDialog::apply);
}

void SettingsDialog::apply() {
  Settings::ChordPreview::set(ui->chordPreviewCheck->isChecked());
  Settings::CustomStyle::set(ui->customStyleCheck->isChecked());
  Settings::SpacebarStop::set(ui->pauseReseekCheck->isChecked());
  Settings::VelocityDrag::set(ui->velocityDragCheck->isChecked());
  Settings::SwapScrollOrientation::set(
      ui->swapScrollOrientationCheck->isChecked());
  Settings::SwapZoomOrientation::set(ui->swapZoomOrientationCheck->isChecked());
}

SettingsDialog::~SettingsDialog() { delete ui; }