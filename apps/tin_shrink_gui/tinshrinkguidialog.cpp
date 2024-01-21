#include "tinshrinkguidialog.h"
#include "ui_tinshrinkguidialog.h"

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QPushButton>
#include <QStringList>

#include <vtkPolyData.h>

#include <guibase/widget/waitdialog.h>
#include <misc/stringtool.h>
#include <tinshrink/tinshrink_main.h>

TinShrinkGuiDialog::TinShrinkGuiDialog(QWidget *parent) :
	QDialog(parent),
	ui(new Ui::TinShrinkGuiDialog)
{
	ui->setupUi(this);
	ui->outputEdit->setSaveMode(true);

	connect(ui->inputEdit, &FilenameEditWidget::changed, this, &TinShrinkGuiDialog::checkInput);
	connect(ui->outputEdit, &FilenameEditWidget::changed, this, &TinShrinkGuiDialog::checkInput);
	connect(ui->checkButton, &QPushButton::clicked, this, &TinShrinkGuiDialog::check);

	checkInput();
}

TinShrinkGuiDialog::~TinShrinkGuiDialog()
{
	delete ui;
}

void TinShrinkGuiDialog::accept()
{
	WaitDialog waitDialog(this);
	waitDialog.disableCancelButton();
	waitDialog.show();

	qApp->processEvents();

	auto input = ui->inputEdit->filename();
	auto output = ui->outputEdit->filename();
	auto interval = ui->intervalSpinBox->value();
	auto distThre1 = ui->distThreshold1SpinBox->value();
	auto distThre2 = ui->distThreshold2SpinBox->value();
	auto angleThre = ui->angleThresholdSpinBox->value();

	tinshrink_main(iRIC::toStr(input), iRIC::toStr(output), interval, distThre1, distThre2, angleThre);

	QDialog::accept();
}

void TinShrinkGuiDialog::checkInput()
{
	QStringList errors;

	if (ui->inputEdit->filename() == "") {
		errors.push_back(tr("Input file not specified."));
	} else {
		QFile input(ui->inputEdit->filename());
		if (! input.exists()) {
			errors.push_back(tr("Input file %1 does not exist.").arg(QDir::toNativeSeparators(ui->inputEdit->filename())));
		}
	}

	if (ui->outputEdit->filename() == "") {
		errors.push_back(tr("Output file not specified."));
	} else {
		QFileInfo outputInfo(ui->outputEdit->filename());
		if (! outputInfo.dir().exists()) {
			errors.push_back(tr("Output folder %1 does not exist.").arg(QDir::toNativeSeparators(outputInfo.dir().absolutePath())));
		}
	}
	ui->errorMessageLabel->setText(errors.join("\n"));

	ui->checkButton->setDisabled(errors.size() > 0);
	ui->buttonBox->button(QDialogButtonBox::Ok)->setDisabled(errors.size() > 0);
}

void TinShrinkGuiDialog::check()
{
	WaitDialog waitDialog(this);
	waitDialog.disableCancelButton();
	waitDialog.show();

	qApp->processEvents();

	auto input = ui->inputEdit->filename();
	auto interval = ui->intervalSpinBox->value();
	auto distThre1 = ui->distThreshold1SpinBox->value();
	auto distThre2 = ui->distThreshold2SpinBox->value();
	auto angleThre = ui->angleThresholdSpinBox->value();

	int inputNumPoints, outputNumPoints;

	auto tin = tinshrink(iRIC::toStr(input), interval, distThre1, distThre2, angleThre, &inputNumPoints, &outputNumPoints);

	ui->inputFileNumPointsValueLabel->setText(QString::number(inputNumPoints));
	ui->outputFileNumPointsValueLabel->setText(QString::number(outputNumPoints));

	if (tin != nullptr) {
		tin->Delete();
	}
}
