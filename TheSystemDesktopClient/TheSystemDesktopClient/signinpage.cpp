#include "signinpage.h"
#include "ui_signinpage.h"
#include "packetmanager.h"

SignInPage::SignInPage(QWidget *parent) :
    Page(parent),
    ui(new Ui::SignInPage)
{
    ui->setupUi(this);

    connect(ui->signinButton, &QPushButton::clicked, this, [=]() { PacketManager::getInstance()->sendSignInPacket(ui->usernameInput->text(), ui->passwordInput->text()); });
}

SignInPage::~SignInPage()
{
    delete ui;
}
