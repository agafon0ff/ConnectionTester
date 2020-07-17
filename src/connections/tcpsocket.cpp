#include "tcpsocket.h"
#include <QHostAddress>

TcpSocket::TcpSocket(QObject *parent) : NetConnection(parent),
    m_tcpSocket(new QTcpSocket(this))
{
    connect(m_tcpSocket, &QTcpSocket::disconnected, this, &TcpSocket::onSocketDisconnected);
    connect(m_tcpSocket, &QTcpSocket::readyRead, this, &TcpSocket::onSocketReadyRead);
    connect(m_tcpSocket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(onSocketError(QAbstractSocket::SocketError)));

    qDebug()<<"*TcpSocket";
}

TcpSocket::~TcpSocket()
{
    qDebug()<<"~TcpSocket";
}

void TcpSocket::closeSocket()
{
    qDebug()<<"TcpSocket::closeSocket";

    if(m_tcpSocket->state() == QTcpSocket::ConnectedState)
        m_tcpSocket->disconnectFromHost();
    else if(m_tcpSocket->state() == QTcpSocket::ConnectingState)
        m_tcpSocket->abort();
}

void TcpSocket::start()
{
    qDebug()<<"TcpSocket::start";

    closeSocket();
    m_socketError.clear();
    m_tcpSocket->connectToHost(settings().host, settings().port);

    if(m_tcpSocket->waitForConnected(1000))
    {
        emit started();
        emit status("OK: TCP-Socket connected to: " + settings().host + ":" +
                     QString::number(settings().port), StatusOk);
    }
    else
    {
        emit status("ERROR: TCP-Socket could not connect to: " + settings().host +
                     ":" + QString::number(settings().port), StatusError);
    }
}

void TcpSocket::stop()
{
    qDebug()<<"TcpSocket::stop";
    closeSocket();
}

void TcpSocket::sendDatagram(const QByteArray &data, const QString &host, quint16 port)
{
    Q_UNUSED(host);
    Q_UNUSED(port);

    if(m_tcpSocket->state() == QTcpSocket::ConnectedState)
        m_tcpSocket->write(data);
}

void TcpSocket::onSocketDisconnected()
{
    emit stopped();

    if(m_socketError.isEmpty())
        emit status("OK: TCP-Socket stopped", StatusOk);
    else
    {
        emit status(m_socketError, StatusConsole);
        emit status("ERROR: TCP-Socket stopped", StatusError);
    }
}

void TcpSocket::onSocketReadyRead()
{
    QByteArray data = m_tcpSocket->readAll();

    emit datagram(data,
                  QHostAddress(m_tcpSocket->peerAddress().toIPv4Address()).toString(),
                  m_tcpSocket->peerPort());
}

void TcpSocket::onSocketError(QAbstractSocket::SocketError socketError)
{
    qDebug()<<"TcpSocket::onSocketError"<<socketError;

    switch (socketError)
    {
    case QAbstractSocket::RemoteHostClosedError:
        m_socketError = "The remote host closed the connection";
        break;

    case QAbstractSocket::HostNotFoundError:
        break;
        m_socketError = "The remote host is not found";

    case QAbstractSocket::ConnectionRefusedError:
        m_socketError = "Connection refused";
        break;

    default:
        break;
    }
}
