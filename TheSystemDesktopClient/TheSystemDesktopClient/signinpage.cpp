#include "signinpage.h"
#include "ui_signinpage.h"

SignInPage::SignInPage(QWidget *parent) :
    Page(parent),
    ui(new Ui::SignInPage)
{
    ui->setupUi(this);
}

SignInPage::~SignInPage()
{
    delete ui;
}
