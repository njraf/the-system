#ifndef PACKETMANAGER_H
#define PACKETMANAGER_H

#include <vector>

#include <QObject>
#include <QThread>
#include <QUdpSocket>
#include <QNetworkDatagram>


class PacketManager : public QObject
{
    Q_OBJECT
public:
    explicit PacketManager(QObject *parent = nullptr);
    ~PacketManager();

    void sendTestPacket();
    void readTestPacket();

private:
    QHostAddress loadBalancerHost;
    const int REQUEST_TX_PORT = 3579;  // SEND to load balancer
    const int RESPONSE_RX_PORT = 3576; // RECV from load balancer
    QUdpSocket *sock;


signals:
    void packetReceived(QString message);
};

#endif // PACKETMANAGER_H
