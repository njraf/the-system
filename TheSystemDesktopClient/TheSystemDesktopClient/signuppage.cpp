#include "signuppage.h"
#include "ui_signuppage.h"
#include "packetmanager.h"

SignUpPage::SignUpPage(QWidget *parent) :
    Page(parent),
    ui(new Ui::SignUpPage)
{
    ui->setupUi(this);
    setObjectName("signUpPage");

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

        PacketManager::getInstance()->sendSignUpPacket(ui->usernameField->text(), ui->passwordField->text(), ui->firstNameField->text(), ui->lastNameField->text());

        //TODO: check username availability from server (move this out of here to change page in reaction to RSLT packet)


        //TODO: create account with SNUP packet


        //TODO: wait for success RSLT packet (move this out of here to change page in reaction to RSLT packet)
        //TODO: change page to home screen

    });
}

SignUpPage::~SignUpPage()
{
    delete ui;
}
