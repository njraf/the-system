#include "signuppage.h"
#include "ui_signuppage.h"
#include "packetmanager.h"
#include "pagenavigator.h"

SignUpPage::SignUpPage(QWidget *parent) :
    Page(parent),
    ui(new Ui::SignUpPage)
{
    ui->setupUi(this);
    setObjectName("signUpPage");

    const PageNavigator *navigator = PageNavigator::getInstance();
    const PacketManager *packetManager = PacketManager::getInstance();

    // result packet received. change page to home or show error message.
    connect(packetManager, &PacketManager::receivedResult, this, [=](bool success, QString message) { success ? navigator->navigate(PageName::HOME) : ui->errorLabel->setText(message); });

    // send sign up packet
    connect(ui->finishButton, &QPushButton::clicked, this, [=]() {
        ui->errorLabel->clear();
        const bool confirmedPassword = (ui->passwordField->text() == ui->confirmPasswordField->text());
        const bool fieldsAreFilled = (
                    !ui->usernameField->text().isEmpty() &&
                    !ui->passwordField->text().isEmpty() &&
                    !ui->confirmPasswordField->text().isEmpty() &&
                    !ui->firstNameField->text().isEmpty() &&
                    !ui->lastNameField->text().isEmpty()
                    );
        if (!confirmedPassword) {
            ui->errorLabel->setText("Passwords do match");
            return;
        }

        if (!fieldsAreFilled) {
            ui->errorLabel->setText("Some fields were left blank");
            return;
        }

        packetManager->sendSignUpPacket(ui->usernameField->text(), ui->passwordField->text(), ui->firstNameField->text(), ui->lastNameField->text());
    });
}

SignUpPage::~SignUpPage()
{
    delete ui;
}
