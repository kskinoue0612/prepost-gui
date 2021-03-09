#include "post3dwindowcellrangesettingwidget.h"
#include "ui_post3dwindowcellrangesettingwidget.h"
#include "../post3dcellrangesettingcontainer.h"

Post3dWindowCellRangeSettingWidget::Post3dWindowCellRangeSettingWidget(QWidget *parent) :
	QWidget(parent),
	ui(new Ui::Post3dWindowCellRangeSettingWidget)
{
	ui->setupUi(this);
}

Post3dWindowCellRangeSettingWidget::~Post3dWindowCellRangeSettingWidget()
{
	delete ui;
}

void Post3dWindowCellRangeSettingWidget::setZoneData(PostZoneDataContainer* zd)
{

}

void Post3dWindowCellRangeSettingWidget::setSetting(const Post3dCellRangeSettingContainer& setting)
{
	ui->enabledCheckBox->setChecked(setting.enabled);
	ui->iMinSlider->setValue(setting.iMin);
	ui->iMaxSlider->setValue(setting.iMax);
	ui->jMinSlider->setValue(setting.jMin);
	ui->jMaxSlider->setValue(setting.jMax);
	ui->kMinSlider->setValue(setting.kMin);
	ui->kMaxSlider->setValue(setting.kMax);
}
Post3dCellRangeSettingContainer Post3dWindowCellRangeSettingWidget::setting() const
{
	Post3dCellRangeSettingContainer setting;
	setting.enabled = ui->enabledCheckBox->isChecked();
	setting.iMin = ui->iMinSlider->value();
	setting.iMax = ui->iMaxSlider->value();
	setting.jMin = ui->jMinSlider->value();
	setting.jMax = ui->jMaxSlider->value();
	setting.kMin = ui->kMinSlider->value();
	setting.kMax = ui->kMaxSlider->value();

	return setting;
}

void Post3dWindowCellRangeSettingWidget::iMinChanged(int imin)
{
	if (ui->iMaxSlider->value() < imin) {
		ui->iMaxSlider->setValue(imin);
	}
}

void Post3dWindowCellRangeSettingWidget::iMaxChanged(int imax)
{
	if (ui->iMinSlider->value() > imax) {
		ui->iMinSlider->setValue(imax);
	}
}

void Post3dWindowCellRangeSettingWidget::jMinChanged(int jmin)
{
	if (ui->jMaxSlider->value() < jmin) {
		ui->jMaxSlider->setValue(jmin);
	}
}

void Post3dWindowCellRangeSettingWidget::jMaxChanged(int jmax)
{
	if (ui->jMinSlider->value() > jmax) {
		ui->jMinSlider->setValue(jmax);
	}
}

void Post3dWindowCellRangeSettingWidget::kMinChanged(int kmin)
{
	if (ui->kMaxSlider->value() < kmin) {
		ui->kMaxSlider->setValue(kmin);
	}
}

void Post3dWindowCellRangeSettingWidget::kMaxChanged(int kmax)
{
	if (ui->kMinSlider->value() > kmax) {
		ui->kMinSlider->setValue(kmax);
	}
}
