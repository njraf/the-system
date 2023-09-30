#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "packetmanager.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    connect(ui->sendButton, &QPushButton::clicked, this, [=]() { packetmanager.sendTestPacket(); });
    connect(&packetmanager, &PacketManager::packetReceived, this, [=](QString message) {
        ui->resultLabel->setText(message);
    });

    packetmanager.start();
}

MainWindow::~MainWindow()
{
    packetmanager.stop();
    packetmanager.wait();
    delete ui;
}

