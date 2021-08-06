#include "tcpserver.h"
#include <QHostAddress>

const char* KEY_SOKET_ID = "socketId";
const char* KEY_SOKET_PORT = "socketPort";
const char* KEY_SOKET_HOST = "socketHost";

TcpServer::TcpServer(QObject *parent) : NetConnection(parent),
    m_tcpServer(new QTcpServer(this))
{
    qDebug() << Q_FUNC_INFO;
}

TcpServer::~TcpServer()
{
    qDebug() << Q_FUNC_INFO;
    closeServer();
}

void TcpServer::closeServer()
{
    if(m_tcpServer->isListening())
    {
        qDebug() << Q_FUNC_INFO;

        disconnect(m_tcpServer, &QTcpServer::newConnection, this, &TcpServer::onSocketConnected);

        QMapIterator<QString, QTcpSocket*> it(m_tcpSockets);
        while (it.hasNext())
        {
            it.next();
            it.value()->disconnectFromHost();
        }

        m_tcpServer->close();
        m_tcpSockets.clear();
    }
}

void TcpServer::start()
{
    qDebug() << Q_FUNC_INFO;

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
    qDebug() << Q_FUNC_INFO;

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
    qDebug() << Q_FUNC_INFO << id;
}

void TcpServer::onSocketDisconnected()
{
    QTcpSocket *socket = dynamic_cast<QTcpSocket*>(sender());
    if (!socket)return;

    QString id = socket->property(KEY_SOKET_ID).toString();
    m_tcpSockets.remove(id);
    socket->deleteLater();

    emit status("Disconnected: " + id, StatusConsole);
    qDebug() << Q_FUNC_INFO << id;
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
