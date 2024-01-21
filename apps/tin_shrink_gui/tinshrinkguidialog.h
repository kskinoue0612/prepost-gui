#ifndef TINSHRINKGUIDIALOG_H
#define TINSHRINKGUIDIALOG_H

#include <QDialog>

namespace Ui {
class TinShrinkGuiDialog;
}

class TinShrinkGuiDialog : public QDialog
{
	Q_OBJECT

public:
	explicit TinShrinkGuiDialog(QWidget *parent = nullptr);
	~TinShrinkGuiDialog();

public slots:
	void accept() override;

private slots:
	void checkInput();
	void check();

private:
	Ui::TinShrinkGuiDialog *ui;
};

#endif // TINSHRINKGUIDIALOG_H
