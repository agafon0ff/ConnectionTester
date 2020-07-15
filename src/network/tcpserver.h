#ifndef TCPSERVER_H
#define TCPSERVER_H

#include "netconnection.h"
#include "global.h"

#include <QTcpServer>
#include <QTcpSocket>

class TcpServer : public NetConnection
{
    Q_OBJECT
public:
    explicit TcpServer(QObject *parent = Q_NULLPTR);
    ~TcpServer();

private:
    QTcpServer *m_tcpServer;
    QMap<QString, QTcpSocket*> m_tcpSockets;

    void closeServer();

public slots:
    void start();
    void stop();

    void sendDatagram(const QByteArray &data, const QString &host, quint16 port);

    int type(){return ConnectionTcpServer;}
    QString getTypeString(){return "TCP-Server";}

private slots:
    void onSocketConnected();
    void onSocketDisconnected();
    void onSocketReadyRead();
};

#endif // TCPSERVER_H
