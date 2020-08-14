#include "tcpserver.h"
#include <QHostAddress>

const char* KEY_SOKET_ID = "socketId";
const char* KEY_SOKET_PORT = "socketPort";
const char* KEY_SOKET_HOST = "socketHost";

TcpServer::TcpServer(QObject *parent) : NetConnection(parent),
    m_tcpServer(new QTcpServer(this))
{
    qDebug()<<"*TcpServer";
}

TcpServer::~TcpServer()
{
    qDebug()<<"~TcpServer";
    closeServer();
}

void TcpServer::closeServer()
{
    if(m_tcpServer->isListening())
    {
        qDebug()<<"TcpServer::closeServer";

        disconnect(m_tcpServer, &QTcpServer::newConnection, this, &TcpServer::onSocketConnected);

        for (int i=0; i<m_tcpSockets.size(); ++i)
            m_tcpSockets.values().at(i)->disconnectFromHost();

        m_tcpServer->close();
        m_tcpSockets.clear();
    }
}

void TcpServer::start()
{
    qDebug()<<"TcpServer::start";

    closeServer();

    const quint16 port = static_cast<quint16>(settings().value(KEY_PORT).toInt());

    if (m_tcpServer->listen(QHostAddress::AnyIPv4, port))
    {
        connect(m_tcpServer, &QTcpServer::newConnection, this, &TcpServer::onSocketConnected);
        emit started();
        emit status("OK: TCP-Server is started on port: " +
                     QString::number(port), StatusOk);
    }
    else
    {
        emit status("ERROR: TCP-Server is not started on port: " +
                     QString::number(port), StatusError);
    }
}

void TcpServer::stop()
{
    qDebug()<<"TcpServer::stop";

    closeServer();

    emit stopped();
    emit status("OK: TCP-Server stopped", StatusOk);
}

void TcpServer::sendDatagram(const QByteArray &data, const QString &host, quint16 port)
{
    if(host.isEmpty())
    {
        for(int i=0; i<m_tcpSockets.size(); ++i)
            m_tcpSockets[m_tcpSockets.keys().at(i)]->write(data);
    }
    else
    {
        QString id = host + ":" + QString::number(port);
        if(m_tcpSockets.contains(id))
            m_tcpSockets.value(id)->write(data);
    }
}

void TcpServer::onSocketConnected()
{
    QTcpSocket* socket = m_tcpServer->nextPendingConnection();

    quint16 port = socket->peerPort();
    QString host = QHostAddress(socket->peerAddress().toIPv4Address()).toString();
    QString id =  host + ":" + QString::number(port);

    connect(socket, &QTcpSocket::disconnected, this, &TcpServer::onSocketDisconnected);
    connect(socket, &QTcpSocket::readyRead, this, &TcpServer::onSocketReadyRead);

    socket->setProperty(KEY_SOKET_ID, id);
    socket->setProperty(KEY_SOKET_HOST, host);
    socket->setProperty(KEY_SOKET_PORT, port);

    m_tcpSockets.insert(id, socket);

    emit status("Connected: " + id, StatusConsole);
    qDebug() << "TcpServer::onSocketConnected" << id;
}

void TcpServer::onSocketDisconnected()
{
    QTcpSocket *socket = dynamic_cast<QTcpSocket*>(sender());
    if (!socket)return;

    QString id = socket->property(KEY_SOKET_ID).toString();
    m_tcpSockets.remove(id);
    socket->deleteLater();

    emit status("Disconnected: " + id, StatusConsole);
    qDebug() << "TcpServer::onSocketDisconnected" << id;
}

void TcpServer::onSocketReadyRead()
{
    QTcpSocket *socket = dynamic_cast<QTcpSocket*>(sender());
    if (!socket)return;

    QByteArray data = socket->readAll();

    emit datagram(data,
                  socket->property(KEY_SOKET_HOST).toString(),
                  static_cast<quint16>(socket->property(KEY_SOKET_PORT).toInt()));
}
