#ifndef PACKETMANAGER_H
#define PACKETMANAGER_H

#include <vector>

#include <QObject>
#include <QThread>
#include <QUdpSocket>
#include <QNetworkDatagram>


class PacketManager : public QThread
{
    Q_OBJECT
public:
    explicit PacketManager(QObject *parent = nullptr);
    ~PacketManager();

    void sendTestPacket();
    void readTestPacket();
    void stop();

protected:
    void run();

private:
    QHostAddress loadBalancerHost;
    QHostAddress desktopClientHost;
    const int REQUEST_TX_PORT = 3579;  // SEND to load balancer
    const int RESPONSE_RX_PORT = 3576; // RECV from load balancer
    //QUdpSocket *sock;
    int sock;
    volatile bool isRunning = true;


signals:
    void packetReceived(QString message);
};

#endif // PACKETMANAGER_H
