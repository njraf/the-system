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

PacketManager* PacketManager::instance = nullptr;

PacketManager::PacketManager(QObject *parent)
    : QThread(parent)
    , sessionID(0)
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
    struct sockaddr_in responseAddr;
    responseAddr.sin_family = AF_INET;
    responseAddr.sin_port = RESPONSE_RX_PORT;
    if (1 != inet_pton(AF_INET, desktopClientHost.toString().toStdString().c_str(), &(responseAddr.sin_addr))) {
        qDebug() << "Response inet_pton error";
    }
    if (-1 == bind(sock, (struct sockaddr*)&responseAddr, sizeof(responseAddr))) {
        qDebug() << "Bind error";
    }

    requestAddr.sin_family = AF_INET;
    requestAddr.sin_port = REQUEST_TX_PORT;
    if (-1 == inet_pton(AF_INET, loadBalancerHost.toString().toStdString().c_str(), &(requestAddr.sin_addr))) {
        qDebug() << "Request inet_pton error";
    }
#endif
}

PacketManager::~PacketManager() {
    isRunning = false;
    close(sock);
    //sock->close();
    //delete sock;
}

PacketManager* PacketManager::getInstance() {
    if (nullptr == PacketManager::instance) {
        instance = new PacketManager();
    }
    return instance;
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

        uint8_t buff[MTU];
        memset(buff, 0, MTU);
        int bytesRead = recv(sock, buff, MTU, 0);
        if (-1 == bytesRead) {
            qDebug() << "Read error";
            errno = 0;
            continue;
        }

        uint8_t *packetPtr = buff + 16;
        char packetType[5] = "";
        memcpy(packetType, packetPtr, 4);
        packetType[4] = '\0';
        if ("RSLT" == QString(packetType)) {
            packetPtr = buff + 28;
            uint32_t success = 0;
            memcpy(&success, packetPtr, sizeof(uint32_t));
            success = ntohl(success);
            packetPtr += 4;
            char message[64] = "";
            memcpy(message, packetPtr, 64);
            qDebug() << "Read" << bytesRead << "bytes\nPacket type: " << packetType << "\nSuccess: " << success << "\nMessage:" << message;
            emit packetReceived(QString(message));
        }
    }
    qDebug() << "Ending thread";
}

void PacketManager::stop() {
    isRunning = false;
}


void PacketManager::sendTestPacket() {
    qDebug() << "Sending message";

    uint8_t packet[MTU];
    uint8_t *packetPtr = packet;
    memcpy(packetPtr, desktopClientHost.toString().toStdString().c_str(), 16);
    packetPtr += 16;
    std::string message = "Hello world";
    memcpy(packetPtr, message.c_str(), (MTU - 16));

    // use Qt sockets //

//    QByteArray messageBytes = "asdzxc";
//    auto byteswrote = sock->writeDatagram(messageBytes, loadBalancerHost, REQUEST_TX_PORT);
//    //auto byteswrote = sock->write(message.c_str(), strlen(message.c_str()));
//    sock->waitForBytesWritten();
//    qDebug() << "Bytes wrote " << byteswrote;


    // use Berkley sockets (works on linux) //
    int bytesWrote = sendto(sock, packet, sizeof(packet), 0, (struct sockaddr*)&requestAddr, sizeof(requestAddr));
    qDebug() << "Bytes wrote" << bytesWrote;
    if (-1 == bytesWrote) {
        qDebug() << "Send error";
        errno = 0;
    }
}

void PacketManager::readTestPacket() {
    qDebug() << "Receiving packet";
    //while (sock->hasPendingDatagrams()) {
    //    QNetworkDatagram datagram = sock->receiveDatagram();
    //    emit packetReceived(QString(datagram.data().data()));
    //}
}

void PacketManager::packHeader(uint8_t *buff, std::string type) {
    uint8_t *buffPtr = buff;
    memcpy(buffPtr, desktopClientHost.toString().toStdString().c_str(), 16);
    buffPtr += 16;
    memcpy(buffPtr, type.c_str(), 4);
    buffPtr += 4;
    uint32_t val32 = htonl(sessionID);
    memcpy(buffPtr, &val32, 4);
    buffPtr += 4;
    //TODO: crc below here
    val32 = htonl(0);
    memcpy(buffPtr, &val32, 4);
    buffPtr += 4;
}

void PacketManager::sendSignInPacket(QString username, QString password) {
    qDebug() << "Sending sign in packet";
    // pack
    //constexpr int PACKET_SIZE = HEADER_SIZE + 128;
    uint8_t packet[MTU];
    memset(packet, 0, MTU);
    uint8_t *packetPtr = packet + HEADER_SIZE;
    memcpy(packetPtr, username.toStdString().c_str(), 64);
    packetPtr += 64;
    memcpy(packetPtr, password.toStdString().c_str(), 64);
    packetPtr += 64;

    packHeader(packet, "SNIN");

    // send
    int bytesWrote = sendto(sock, packet, sizeof(packet), 0, (struct sockaddr*)&requestAddr, sizeof(requestAddr));
    qDebug() << "Bytes wrote" << bytesWrote;
    if (-1 == bytesWrote) {
        qDebug() << "Send error";
        errno = 0;
    }
}

