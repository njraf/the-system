#include "packetmanager.h"

#include <algorithm>

#include <QFile>
#include <QStringList>

//#ifndef _WIN32
//#include <sys/socket.h>
//#include <sys/types.h>
//#include <sys/select.h>
//#include <arpa/inet.h>
//#include <unistd.h>
//#include <netdb.h>
//#include <netinet/in.h>
//#endif

PacketManager* PacketManager::instance = nullptr;

PacketManager::PacketManager(QObject *parent)
    : QThread(parent)
    , sessionID(0)
    //, sock(new QUdpSocket())
{
    // get IP addresses of the load balancer and this client
    QFile file("../../addresses.csv");
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "Failed to set IP addresses" << file.errorString();
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
    sock = createSocket(AF_INET, SOCK_DGRAM, 0);
    if (INVALID_SOCKET == sock) {
        qDebug() << "Bad sock fd";
        return;
    }
    struct sockaddr_in responseAddr;
    makeSockaddr(responseAddr, AF_INET, desktopClientHost.toString().toStdString().c_str(), RESPONSE_RX_PORT);
    if (-1 == bind(sock, (struct sockaddr*)&responseAddr, sizeof(responseAddr))) {
        qDebug() << "Bind error";
        return;
    }

    makeSockaddr(requestAddr, AF_INET, loadBalancerHost.toString().toStdString().c_str(), REQUEST_TX_PORT);
}

PacketManager::~PacketManager() {
    isRunning = false;
    closeSocket(sock);
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
        int bytesRead = recv(sock, (char*)buff, MTU, 0);
        if (-1 == bytesRead) {
            qDebug() << "Read error";
            errno = 0;
            continue;
        }

        PacketHeader packetHeader;
        if (!unpackHeader(buff, packetHeader)) {
            continue;
        }
        uint8_t *packetPtr = buff + 16;
        char packetType[5] = "";
        memcpy(packetType, packetHeader.packetType, 4);
        packetType[4] = '\0';
        if ("RSLT" == QString(packetType)) {
            packetPtr = buff + 28;
            uint32_t val32 = 0;
            memcpy(&val32, packetPtr, sizeof(uint32_t));
            bool success = static_cast<bool>(ntohl(val32));
            packetPtr += sizeof(uint32_t);
            char message[STD_STRING_LENGTH] = "";
            memcpy(message, packetPtr, STD_STRING_LENGTH);
            qDebug() << "Read" << bytesRead << "bytes\nPacket type: " << packetType << "\nSuccess: " << success << "\nMessage:" << message;
            emit receivedResult(success, QString(message));
        }
    }
    qDebug() << "Ending thread";
}

void PacketManager::stop() {
    isRunning = false;
}

void PacketManager::packHeader(uint8_t *buff, std::string type) const {
    uint8_t *buffPtr = buff;
    memcpy(buffPtr, desktopClientHost.toString().toStdString().c_str(), 16);
    buffPtr += 16;
    memcpy(buffPtr, type.c_str(), 4);
    buffPtr += 4;
    uint32_t val32 = htonl(sessionID);
    memcpy(buffPtr, &val32, sizeof(uint32_t));
    buffPtr += sizeof(uint32_t);
    //TODO: crc below here
    val32 = htonl(0);
    memcpy(buffPtr, &val32, sizeof(uint32_t));
    buffPtr += sizeof(uint32_t);
}

bool PacketManager::unpackHeader(uint8_t *buff, PacketHeader &header) {
    uint8_t *buffPtr = buff;
    memcpy(header.ipAddress, buffPtr, 16);
    buffPtr += 16;
    memcpy(header.packetType, buffPtr, 4);
    buffPtr += 4;
    uint32_t val32 = 0;
    memcpy(&val32, buffPtr, sizeof(uint32_t));
    header.sessionID = ntohl(val32);
    buffPtr += sizeof(uint32_t);
    memcpy(&val32, buffPtr, sizeof(uint32_t));
    header.crc = ntohl(val32);
    buffPtr += sizeof(uint32_t);

    //TODO: check crc


    // check that we recieved the correct session ID
    if (0 == sessionID) {
        sessionID = header.sessionID;
    } else if (header.sessionID != sessionID) {
        qDebug() << "ERROR: Packet with bad session ID received";
        return false;
    }

    // check that the header IP address matches the one we sent
    if (QString(header.ipAddress) != desktopClientHost.toString()) {
        qDebug() << "ERROR: The returned IP address of the received packet is different than our own";
        return false;
    }
    return true;
}

void PacketManager::sendSignInPacket(QString username, QString password) const {
    qDebug() << "Sending sign in packet";
    // pack
    //constexpr int PACKET_SIZE = HEADER_SIZE + 128;
    uint8_t packet[MTU];
    memset(packet, 0, MTU);
    uint8_t *packetPtr = packet + HEADER_SIZE;
    memcpy(packetPtr, username.toStdString().c_str(), STD_STRING_LENGTH);
    packetPtr += STD_STRING_LENGTH;
    memcpy(packetPtr, password.toStdString().c_str(), STD_STRING_LENGTH);
    packetPtr += STD_STRING_LENGTH;

    packHeader(packet, "SNIN");

    // send
    int bytesWrote = sendto(sock, (char*)packet, sizeof(packet), 0, (struct sockaddr*)&requestAddr, sizeof(requestAddr));
    qDebug() << "Bytes wrote" << bytesWrote;
    if (-1 == bytesWrote) {
        qDebug() << "Send error";
        errno = 0;
    }
}

void PacketManager::sendSignUpPacket(QString username, QString password, QString firstName, QString lastName) const {
    qDebug() << "Sending sign up packet";

    uint8_t packet[MTU];
    memset(packet, 0, MTU);
    uint8_t *packetPtr = packet + HEADER_SIZE;
    memcpy(packetPtr, username.toStdString().c_str(), STD_STRING_LENGTH);
    packetPtr += STD_STRING_LENGTH;
    memcpy(packetPtr, password.toStdString().c_str(), STD_STRING_LENGTH);
    packetPtr += STD_STRING_LENGTH;
    memcpy(packetPtr, firstName.toStdString().c_str(), STD_STRING_LENGTH);
    packetPtr += STD_STRING_LENGTH;
    memcpy(packetPtr, lastName.toStdString().c_str(), STD_STRING_LENGTH);
    packetPtr += STD_STRING_LENGTH;

    packHeader(packet, "SNUP");

    // send
    int bytesWrote = sendto(sock, (char*)packet, sizeof(packet), 0, (struct sockaddr*)&requestAddr, sizeof(requestAddr));
    qDebug() << "Bytes wrote" << bytesWrote;
    if (-1 == bytesWrote) {
        qDebug() << "Send error";
        errno = 0;
    }
}
