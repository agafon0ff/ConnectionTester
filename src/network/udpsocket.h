#ifndef UDPSOCKET_H
#define UDPSOCKET_H

#include "netconnection.h"
#include "global.h"

#include <QUdpSocket>

class UdpSocket : public NetConnection
{
    Q_OBJECT
public:
    explicit UdpSocket(QObject *parent = Q_NULLPTR);
    ~UdpSocket();

private:
    QUdpSocket *m_udpSocket;

    void closeSocket();
public slots:
    void start();
    void stop();

    void sendDatagram(const QByteArray &data, const QString &host, quint16 port);

    int type(){return ConnectionUdpSocket;}
    QString getTypeString(){return "UDP-Socket";}

private slots:
    void onSocketReadyRead();
};

#endif // UDPSOCKET_H
