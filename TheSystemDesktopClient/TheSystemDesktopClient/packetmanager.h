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


protected:
    void run();

private:
    static PacketManager *instance;
    static constexpr int HEADER_SIZE = 28;
    static constexpr int MTU = 1500;

    QHostAddress loadBalancerHost;
    QHostAddress desktopClientHost;
    const int REQUEST_TX_PORT = 3579;  // SEND to load balancer
    const int RESPONSE_RX_PORT = 3576; // RECV from load balancer
    struct sockaddr_in requestAddr;
    //QUdpSocket *sock;
    int sock;
    volatile bool isRunning = true;
    int sessionID;

    void packHeader(uint8_t *buff, std::string type) const; // NOTE: call at the end of each pack function for accurate CRC value


signals:
    void packetReceived(QString message);
    void receivedResult(bool success, QString message);
};

#endif // PACKETMANAGER_H
