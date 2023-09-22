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

    void removeStoppedThreads(); // stopped means finished

private:
    class SocketManager : public QThread
    {
    public:
        SocketManager(QUdpSocket *sock);
        ~SocketManager();



    protected:
        void run();

    private:
        QUdpSocket *sock;
    };

    QHostAddress loadBalancerHost;
    const int REQUEST_TX_PORT = 3579;  // SEND to load balancer
    const int RESPONSE_RX_PORT = 3579; // RECV from load balancer
    QUdpSocket *sock;

    std::vector<SocketManager*> socketManagers;



signals:
    void packetReceived(QString message);
};

#endif // PACKETMANAGER_H
