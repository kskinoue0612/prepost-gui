#include "gridcomplexconditiongroup.h"
#include "gridcomplexconditiongroupeditdialog.h"
#include "ui_gridcomplexconditiongroupeditdialog.h"

GridComplexConditionGroupEditDialog::GridComplexConditionGroupEditDialog(QWidget *parent) :
	QDialog(parent),
	ui(new Ui::GridComplexConditionGroupEditDialog),
	m_backup {nullptr}
{
	ui->setupUi(this);
}

GridComplexConditionGroupEditDialog::~GridComplexConditionGroupEditDialog()
{
	delete ui;
	delete m_backup;
}

void GridComplexConditionGroupEditDialog::setGroup(GridComplexConditionGroup* group)
{
	m_group = group;
	m_backup = group->containerSet()->clone();

	ui->editWidget->setGroup(group);
}

void GridComplexConditionGroupEditDialog::reject()
{
	m_group->containerSet()->copyValues(m_backup);
	QDialog::reject();
}
