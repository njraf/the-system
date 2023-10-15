#include <QMap>
#include <QDebug>

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "packetmanager.h"
#include "pagenavigator.h"
#include "signinpage.h"
#include "signuppage.h"
#include "homepage.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    PageNavigator *pageNavigator = PageNavigator::getInstance();
    connect(pageNavigator, &PageNavigator::pageChanged, this, [=](QSharedPointer<Page> page) {
        const int stackIndex = ui->pages->addWidget(page.data());
        ui->pages->setCurrentIndex(stackIndex);
    });

    QMap<PageName, std::function<QSharedPointer<Page>(void)>> routes;
    routes.insert(PageName::SIGN_IN, []() { return QSharedPointer<SignInPage>::create(); });
    routes.insert(PageName::SIGN_UP, []() { return QSharedPointer<SignUpPage>::create(); });
    routes.insert(PageName::HOME, []() { return QSharedPointer<HomePage>::create(); });

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

