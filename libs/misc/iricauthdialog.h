#ifndef IRICAUTHDIALOG_H
#define IRICAUTHDIALOG_H

#include "misc_global.h"

#include <QDialog>

class iRICAuthClient;

namespace Ui
{
	class iRICAuthDialog;
}

/// Sign-in dialog for iRIC ID.
///
/// Shown at startup when a silent login is not possible, and also from the
/// Help menu. The dialog only borrows the iRICAuthClient; it does not own it.
/// Closing the dialog always lets iRIC continue - signing in is optional.
class MISCDLL_EXPORT iRICAuthDialog : public QDialog
{
	Q_OBJECT

public:
	explicit iRICAuthDialog(iRICAuthClient* client, QWidget* parent = nullptr);
	~iRICAuthDialog();

private slots:
	void startSignIn();
	void handleLoginSucceeded();
	void handleLoginFailed(const QString& reason);
	void updateState();

private:
	iRICAuthClient* m_client;
	Ui::iRICAuthDialog* ui;
};

#endif // IRICAUTHDIALOG_H
