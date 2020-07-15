#ifndef TCPSOCKET_H
#define TCPSOCKET_H

#include "netconnection.h"
#include "global.h"

#include <QTcpSocket>

class TcpSocket : public NetConnection
{
    Q_OBJECT
public:
    explicit TcpSocket(QObject *parent = Q_NULLPTR);
    ~TcpSocket();

private:
    QTcpSocket *m_tcpSocket;
    QString m_socketError;

    void closeSocket();

public slots:
    void start();
    void stop();

    void sendDatagram(const QByteArray &data, const QString &host, quint16 port);

    int type(){return ConnectionTcpSocket;}
    QString getTypeString(){return "TCP-Socket";}

private slots:
    void onSocketDisconnected();
    void onSocketReadyRead();
    void onSocketError(QAbstractSocket::SocketError socketError);
};

#endif // TCPSOCKET_H
