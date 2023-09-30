#include "packetmanager.h"

#include <algorithm>

#include <QFile>
#include <QStringList>

#ifndef _WIN32
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/select.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>
#include <netinet/in.h>
#endif


PacketManager::PacketManager(QObject *parent)
    : QThread(parent)
    //, sock(new QUdpSocket())
{
    // get IP addresses of the load balancer and this client
    QFile file("../../addresses.csv");
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "Failed to set IP addresses";
        return;
    }

    while (!file.atEnd()) {
        const QStringList line = QString(file.readLine().toStdString().c_str()).split(',');
        if (line[0] == "load_balancer") {
            qDebug() << "Setting Load Balancer IP address to" << line[1].trimmed();
            loadBalancerHost = QHostAddress(line[1].trimmed());
        } else if (line[0] == "desktop_client") {
            qDebug() << "Setting own IP address to" << line[1].trimmed();
            desktopClientHost = QHostAddress(line[1].trimmed());
        }
    }
    file.close();

    // Qt socket //
    //QObject::connect(sock, &QUdpSocket::readyRead, this, &PacketManager::readTestPacket);
    //sock->bind(QHostAddress::AnyIPv4, RESPONSE_RX_PORT);


    // Berkley socket //
#ifndef _WIN32
    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (-1 == sock) {
        qDebug() << "Bad sock fd";
    }
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = RESPONSE_RX_PORT;
    if (1 != inet_pton(AF_INET, desktopClientHost.toString().toStdString().c_str(), &(addr.sin_addr))) {
        qDebug() << "inet_pton error";
    }
    if (-1 == bind(sock, (struct sockaddr*)&addr, sizeof(addr))) {
        qDebug() << "Bind error";
    }
#endif
}

PacketManager::~PacketManager() {
    isRunning = false;
    close(sock);
    //sock->close();
    //delete sock;
}

void PacketManager::run() {
    fd_set mainSet;
    FD_ZERO(&mainSet);
    FD_SET(sock, &mainSet);

    while (isRunning) {
        fd_set readSet = mainSet;

        struct timeval tv;
        tv.tv_sec = 1;
        tv.tv_usec = 0;

        int ret = select(sock + 1, &readSet, nullptr, nullptr, &tv);

        if (-1 == ret) {
            // error
            switch (errno) {
            case EBADF:
                qDebug() << "Error: bad FD" << sock;
                break;
            case EINTR:
                qDebug() << "Error: signal caught";
                break;
            case EINVAL:
                qDebug() << "Bad nfds or tv";
                break;
            case ENOMEM:
                qDebug() << "Error: no memory";
                break;
            default:
                qDebug() << "Other error";
                break;
            }

            errno = 0;
            break;
        } else if (0 == ret) {
            // timeout
            continue;
        } else if (!FD_ISSET(sock, &readSet)) {
            continue;
        }

        char buff[64] = "";
        recv(sock, buff, sizeof(buff), 0);
        emit packetReceived(QString(buff));
    }
    qDebug() << "Ending thread";
}

void PacketManager::stop() {
    isRunning = false;
}


void PacketManager::sendTestPacket() {
    qDebug() << "Sending message";

    const std::string message = "The message";

    // use Qt sockets //

//    QByteArray messageBytes = "asdzxc";
//    auto byteswrote = sock->writeDatagram(messageBytes, loadBalancerHost, REQUEST_TX_PORT);
//    //auto byteswrote = sock->write(message.c_str(), strlen(message.c_str()));
//    sock->waitForBytesWritten();
//    qDebug() << "Bytes wrote " << byteswrote;


    // use Berkley sockets (works on linux) //

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = REQUEST_TX_PORT;
    inet_pton(AF_INET, loadBalancerHost.toString().toStdString().c_str(), &(addr.sin_addr));
    int bytesWrote = sendto(sock, message.c_str(), strlen(message.c_str()), 0, (struct sockaddr*)&addr, sizeof(addr));
    qDebug() << "Bytes wrote" << bytesWrote;
}

void PacketManager::readTestPacket() {
    qDebug() << "Receiving packet";
    //while (sock->hasPendingDatagrams()) {
    //    QNetworkDatagram datagram = sock->receiveDatagram();
    //    emit packetReceived(QString(datagram.data().data()));
    //}
}
