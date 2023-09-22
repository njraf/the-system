#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "packetmanager.h"

PacketManager pm;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    connect(ui->sendButton, &QPushButton::clicked, this, [=]() { pm.sendTestPacket(); });
    connect(&packetmanager, &PacketManager::packetReceived, this, [=](QString message) {
        ui->resultLabel->setText(message);
    });
}

MainWindow::~MainWindow()
{
    delete ui;
}

