#include "ui_iricauthdialog.h"

#include "iricauthclient.h"
#include "iricauthdialog.h"

#include <QDialogButtonBox>
#include <QPushButton>

iRICAuthDialog::iRICAuthDialog(iRICAuthClient* client, QWidget* parent) :
	QDialog(parent),
	m_client {client},
	ui {new Ui::iRICAuthDialog}
{
	ui->setupUi(this);

	connect(ui->signInButton, &QPushButton::clicked, this, &iRICAuthDialog::startSignIn);
	connect(ui->cancelButton, &QPushButton::clicked, this, &iRICAuthDialog::cancelSignIn);
	connect(ui->logoutButton, &QPushButton::clicked, this, [this]() {
		if (m_client != nullptr) {m_client->logout();}
	});

	if (m_client != nullptr) {
		connect(m_client, &iRICAuthClient::authorizationCodeReceived, this, &iRICAuthDialog::handleAuthorizationCodeReceived);
		connect(m_client, &iRICAuthClient::loginSucceeded, this, &iRICAuthDialog::handleLoginSucceeded);
		connect(m_client, &iRICAuthClient::loginFailed, this, &iRICAuthDialog::handleLoginFailed);
		connect(m_client, &iRICAuthClient::loggedOut, this, &iRICAuthDialog::handleLoggedOut);
	}

	updateRestingState();
}

iRICAuthDialog::~iRICAuthDialog()
{
	delete ui;
}

void iRICAuthDialog::startSignIn()
{
	if (m_client == nullptr) {return;}

	ui->signInButton->setVisible(true);
	ui->signInButton->setEnabled(false);
	ui->cancelButton->setVisible(true);
	ui->logoutButton->setVisible(false);
	ui->statusLabel->setText(tr("A sign-in page has opened in your web browser. "
								"Complete the sign-in there, then return to iRIC."));
	m_client->startInteractiveLogin();
}

void iRICAuthDialog::cancelSignIn()
{
	if (m_client == nullptr) {return;}

	m_client->cancelInteractiveLogin();
}

void iRICAuthDialog::handleAuthorizationCodeReceived()
{
	ui->cancelButton->setVisible(false);
	ui->statusLabel->setText(tr("Received your sign-in. Contacting the iRIC ID service..."));
}

void iRICAuthDialog::handleLoginSucceeded()
{
	updateRestingState();

	// Turn the Close button into an explicit "OK" and let the user dismiss
	// the dialog when they have read the confirmation (no auto-close).
	QPushButton* closeButton = ui->buttonBox->button(QDialogButtonBox::Close);
	if (closeButton != nullptr) {
		closeButton->setText(tr("OK"));
		closeButton->setDefault(true);
		closeButton->setFocus();
	}
}

void iRICAuthDialog::handleLoginFailed(const QString& reason)
{
	ui->cancelButton->setVisible(false);
	ui->signInButton->setVisible(true);
	ui->signInButton->setEnabled(true);
	ui->logoutButton->setVisible((m_client != nullptr) && m_client->isLoggedIn());
	ui->statusLabel->setText(reason.isEmpty() ? tr("Sign-in failed.") : reason);
}

void iRICAuthDialog::handleLoggedOut()
{
	updateRestingState();
	ui->statusLabel->setText(tr("You are signed out."));
}

void iRICAuthDialog::updateRestingState()
{
	const bool loggedIn = (m_client != nullptr) && m_client->isLoggedIn();

	ui->cancelButton->setVisible(false);
	ui->logoutButton->setVisible(loggedIn);
	ui->signInButton->setVisible(! loggedIn);
	ui->signInButton->setEnabled(! loggedIn);

	if (loggedIn) {
		const QString who = m_client->email().isEmpty() ? m_client->userId() : m_client->email();
		const QString sub = m_client->userId();
		QString text = tr("Signed in as %1.").arg(who);
		if (! sub.isEmpty()) {
			text += tr(" (user_id: %1)").arg(sub);
		}
		ui->statusLabel->setText(text);
	} else {
		ui->statusLabel->setText(tr("Sign in with your iRIC ID so that iRIC can record which user "
									"started it. Signing in is optional - you can skip this and "
									"continue using iRIC."));
	}
}
