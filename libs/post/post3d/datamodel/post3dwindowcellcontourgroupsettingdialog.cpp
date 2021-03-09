#include "post3dwindowcellcontourgroupsettingdialog.h"
#include "ui_post3dwindowcellcontourgroupsettingdialog.h"

Post3dWindowCellContourGroupSettingDialog::Post3dWindowCellContourGroupSettingDialog(QWidget *parent) :
	QDialog(parent),
	ui(new Ui::Post3dWindowCellContourGroupSettingDialog)
{
	ui->setupUi(this);
}

Post3dWindowCellContourGroupSettingDialog::~Post3dWindowCellContourGroupSettingDialog()
{
	delete ui;
}
