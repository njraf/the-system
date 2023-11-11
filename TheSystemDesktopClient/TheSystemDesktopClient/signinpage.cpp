#include "signinpage.h"
#include "ui_signinpage.h"
#include "packetmanager.h"
#include "pagenavigator.h"

SignInPage::SignInPage(QWidget *parent) :
    Page(PageName::SIGN_IN, parent),
    ui(new Ui::SignInPage)
{
    ui->setupUi(this);
    setObjectName("signInPage");

    const PageNavigator *navigator = PageNavigator::getInstance();
    const PacketManager *packetManager = PacketManager::getInstance();

    // send sign in packet
    connect(ui->signinButton, &QPushButton::clicked, this, [=]() { packetManager->sendSignInPacket(ui->usernameInput->text(), ui->passwordInput->text()); });

    // change to sign up page
    connect(ui->signupButton, &QPushButton::clicked, this, [=]() { navigator->navigate(PageName::SIGN_UP); });

    // result packet received. change page or display failed login message
    connect(packetManager, &PacketManager::receivedResult, this, [=](bool success, QString message) { success ? navigator->navigate(PageName::HOME) : ui->statusLabel->setText(message); });
}

SignInPage::~SignInPage()
{
    delete ui;
}
