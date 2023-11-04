#ifndef PACKETMANAGER_H
#define PACKETMANAGER_H

#include <vector>

#include <QObject>
#include <QThread>
#include <QUdpSocket>
#include <QNetworkDatagram>

#ifndef _WIN32
#include <netinet/in.h>
#endif

typedef struct {
    char ipAddress[16];
    char packetType[4];
    uint32_t sessionID;
    uint32_t crc;
} PacketHeader;

class PacketManager : public QThread
{
    Q_OBJECT
public:
    explicit PacketManager(QObject *parent = nullptr);
    ~PacketManager();
    static PacketManager* getInstance();

    void sendTestPacket();
    void readTestPacket();
    void stop();

    // send functions
    void sendSignInPacket(QString username, QString password) const;
    void sendSignUpPacket(QString username, QString password, QString firstName, QString lastName) const;


protected:
    void run();

private:
    static PacketManager *instance;
    static constexpr int HEADER_SIZE = 28;
    static constexpr int MTU = 1500;
    static constexpr int STD_STRING_LENGTH = 64;

    QHostAddress loadBalancerHost;
    QHostAddress desktopClientHost;
    const int REQUEST_TX_PORT = 3579;  // SEND to load balancer
    const int RESPONSE_RX_PORT = 3576; // RECV from load balancer
    struct sockaddr_in requestAddr;
    //QUdpSocket *sock;
    int sock;
    volatile bool isRunning = true;
    uint32_t sessionID;

    void packHeader(uint8_t *buff, std::string type) const; // NOTE: call at the end of each pack function for accurate CRC value
    bool unpackHeader(uint8_t *buff, PacketHeader &header);


signals:
    void packetReceived(QString message);
    void receivedResult(bool success, QString message);
};

#endif // PACKETMANAGER_H
