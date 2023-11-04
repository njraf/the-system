#include "blankpage.h"
#include "ui_blankpage.h"

BlankPage::BlankPage(QWidget *parent) :
    Page(PageName::BLANK, parent),
    ui(new Ui::BlankPage)
{
    ui->setupUi(this);
}

BlankPage::~BlankPage()
{
    delete ui;
}
