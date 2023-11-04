#include "homepage.h"
#include "ui_homepage.h"

HomePage::HomePage(QWidget *parent) :
    Page(PageName::HOME, parent),
    ui(new Ui::HomePage)
{
    ui->setupUi(this);
    setObjectName("homePage");
}

HomePage::~HomePage()
{
    delete ui;
}
