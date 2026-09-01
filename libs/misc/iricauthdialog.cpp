#include "ui_iricauthdialog.h"

#include "iricauthclient.h"
#include "iricauthdialog.h"

#include <QPushButton>
#include <QTimer>

iRICAuthDialog::iRICAuthDialog(iRICAuthClient* client, QWidget* parent) :
	QDialog(parent),
	m_client {client},
	ui {new Ui::iRICAuthDialog}
{
	ui->setupUi(this);

	connect(ui->signInButton, &QPushButton::clicked, this, &iRICAuthDialog::startSignIn);
	connect(ui->logoutButton, &QPushButton::clicked, this, [this]() {
		if (m_client != nullptr) {m_client->logout();}
	});

	if (m_client != nullptr) {
		connect(m_client, &iRICAuthClient::loginSucceeded, this, &iRICAuthDialog::handleLoginSucceeded);
		connect(m_client, &iRICAuthClient::loginFailed, this, &iRICAuthDialog::handleLoginFailed);
		connect(m_client, &iRICAuthClient::stateChanged, this, &iRICAuthDialog::updateState);
	}

	updateState();
}

iRICAuthDialog::~iRICAuthDialog()
{
	delete ui;
}

void iRICAuthDialog::startSignIn()
{
	if (m_client == nullptr) {return;}

	ui->signInButton->setEnabled(false);
	ui->statusLabel->setText(tr("A sign-in page has opened in your web browser. "
								"Complete the sign-in there, then return to iRIC."));
	m_client->startInteractiveLogin();
}

void iRICAuthDialog::handleLoginSucceeded()
{
	updateState();
	// Give the user a moment to see the result, then close.
	QTimer::singleShot(800, this, &QDialog::accept);
}

void iRICAuthDialog::handleLoginFailed(const QString& reason)
{
	ui->signInButton->setEnabled(true);
	ui->statusLabel->setText(reason.isEmpty() ? tr("Sign-in failed.") : reason);
}

void iRICAuthDialog::updateState()
{
	const bool loggedIn = (m_client != nullptr) && m_client->isLoggedIn();

	ui->logoutButton->setVisible(loggedIn);
	ui->signInButton->setVisible(! loggedIn);

	if (loggedIn) {
		const QString who = m_client->email().isEmpty() ? m_client->userId() : m_client->email();
		ui->statusLabel->setText(tr("Signed in as %1.").arg(who));
	} else {
		ui->signInButton->setEnabled(true);
		ui->statusLabel->setText(tr("Sign in with your iRIC ID so that iRIC can record which user "
									"started it. Signing in is optional - you can skip this and "
									"continue using iRIC."));
	}
}
