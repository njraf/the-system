#include "packetmanager.h"

#include <algorithm>

PacketManager::PacketManager(QObject *parent)
    : QObject(parent)
    , sock(new QUdpSocket())
{
    QObject::connect(sock, &QUdpSocket::readyRead, this, &PacketManager::readTestPacket);
    sock->bind(QHostAddress::LocalHost, RESPONSE_RX_PORT);
    loadBalancerHost = QHostAddress::LocalHost;
}

PacketManager::~PacketManager() {
    sock->close();
    delete sock;
}


void PacketManager::sendTestPacket() {
    //removeStoppedThreads();
    //SocketManager *sm = new SocketManager(sock);
    //socketManagers.push_back(sm);
    //sm->start();

    qDebug("Sending message");

    const std::string message = "The message";
    sock->writeDatagram(message.c_str(), sizeof(message.c_str()), loadBalancerHost, REQUEST_TX_PORT);
}

void PacketManager::readTestPacket() {
    auto asd = sock->receiveDatagram();
    while (sock->hasPendingDatagrams()) {
        QNetworkDatagram datagram = sock->receiveDatagram();
        emit packetReceived(QString(datagram.data().data()));
    }
}

void PacketManager::removeStoppedThreads() {
    socketManagers.erase(std::remove_if(socketManagers.begin(), socketManagers.end(), [=](SocketManager *socketManager) {
                             if (socketManager->isFinished()) {
                                 delete socketManager;
                                 return true;
                             } else {
                                 return false;
                             }
    }), socketManagers.end());
}


////////////////////////////////////////////////////////////////


PacketManager::SocketManager::SocketManager(QUdpSocket *sock_)
    : QThread{}
    , sock(sock_)
{

}

PacketManager::SocketManager::~SocketManager() {

}

void PacketManager::SocketManager::run() {
    //readTestPacket();
}
