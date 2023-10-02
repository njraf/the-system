#include <QMap>

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "packetmanager.h"
#include "pagenavigator.h"
#include "signinpage.h"
//#include "signuppage.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    PageNavigator *pageNavigator = PageNavigator::getInstance();
    connect(pageNavigator, &PageNavigator::pageChanged, this, [=](QSharedPointer<Page> page) {
        ui->pages->addWidget(page.data());
    });

    QSharedPointer<SignInPage> signinPage = QSharedPointer<SignInPage>::create(this);
    ui->pages->addWidget(signinPage.data());

    QMap<PageName, std::function<QSharedPointer<Page>(void)>> routes;
    routes.insert(PageName::SIGN_IN, []() { return QSharedPointer<SignInPage>::create(); });
    //routes.insert(PageName::SIGN_UP, []() { return QSharedPointer<SignUpPage>::create(); });

    pageNavigator->populateRoutes(routes);
    pageNavigator->navigate(PageName::SIGN_IN);

    PacketManager::getInstance()->start();
}

MainWindow::~MainWindow()
{
    PacketManager::getInstance()->stop();
    PacketManager::getInstance()->wait();
    delete PageNavigator::getInstance();
    delete ui;
}

