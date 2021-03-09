#ifndef POST3DWINDOWCELLCONTOURGROUPSETTINGDIALOG_H
#define POST3DWINDOWCELLCONTOURGROUPSETTINGDIALOG_H

#include <QDialog>

namespace Ui {
class Post3dWindowCellContourGroupSettingDialog;
}

class Post3dWindowCellContourGroupSettingDialog : public QDialog
{
	Q_OBJECT

public:
	explicit Post3dWindowCellContourGroupSettingDialog(QWidget *parent = nullptr);
	~Post3dWindowCellContourGroupSettingDialog();

private:
	Ui::Post3dWindowCellContourGroupSettingDialog *ui;
};

#endif // POST3DWINDOWCELLCONTOURGROUPSETTINGDIALOG_H
