#include "packetmanager.h"

#include <algorithm>

#ifndef _WIN32
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>
#endif


PacketManager::PacketManager(QObject *parent)
    : QObject(parent)
    , sock(new QUdpSocket())
{
    QObject::connect(sock, &QUdpSocket::readyRead, this, &PacketManager::readTestPacket);
    sock->bind(QHostAddress::Any, RESPONSE_RX_PORT);
    loadBalancerHost = QHostAddress("127.0.0.1");
}

PacketManager::~PacketManager() {
    sock->close();
    delete sock;
}


void PacketManager::sendTestPacket() {
    qDebug("Sending message");

    const std::string message = "The message";

    // use Qt sockets //

//    QByteArray messageBytes = "asdzxc";
//    auto byteswrote = sock->writeDatagram(messageBytes, loadBalancerHost, REQUEST_TX_PORT);
//    //auto byteswrote = sock->write(message.c_str(), strlen(message.c_str()));
//    sock->waitForBytesWritten();
//    qDebug() << "Bytes wrote " << byteswrote;


    // use Berkley sockets (works on linux) //

#ifndef _WIN32
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = REQUEST_TX_PORT;
    inet_pton(AF_INET, loadBalancerHost.toString().toStdString().c_str(), &(addr.sin_addr));
    int bytesWrote = sendto(sock, message.c_str(), strlen(message.c_str()), 0, (struct sockaddr*)&addr, sizeof(addr));
    qDebug() << "Bytes wrote " << bytesWrote;
    close(sock);
#endif
}

void PacketManager::readTestPacket() {
    qDebug() << "Receiving packet";
    while (sock->hasPendingDatagrams()) {
        QNetworkDatagram datagram = sock->receiveDatagram();
        emit packetReceived(QString(datagram.data().data()));
    }
}
