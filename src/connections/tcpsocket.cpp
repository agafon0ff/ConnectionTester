#include "tcpsocket.h"
#include <QHostAddress>

TcpSocket::TcpSocket(QObject *parent) : NetConnection(parent),
    m_tcpSocket(new QTcpSocket(this))
{
    connect(m_tcpSocket, &QTcpSocket::disconnected, this, &TcpSocket::onSocketDisconnected);
    connect(m_tcpSocket, &QTcpSocket::readyRead, this, &TcpSocket::onSocketReadyRead);
    connect(m_tcpSocket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(onSocketError(QAbstractSocket::SocketError)));

    qDebug() << Q_FUNC_INFO;
}

TcpSocket::~TcpSocket()
{
    qDebug() << Q_FUNC_INFO;
}

void TcpSocket::closeSocket()
{
    qDebug() << Q_FUNC_INFO;

    if(m_tcpSocket->state() == QTcpSocket::ConnectedState)
        m_tcpSocket->disconnectFromHost();
    else if(m_tcpSocket->state() == QTcpSocket::ConnectingState)
        m_tcpSocket->abort();
}

void TcpSocket::start()
{
    qDebug() << Q_FUNC_INFO;

    closeSocket();

    const QString host = settings().value(KEY_HOST).toString();
    const quint16 port = static_cast<quint16>(settings().value(KEY_PORT).toInt());

    m_socketError.clear();
    m_tcpSocket->connectToHost(host, port);

    if(m_tcpSocket->waitForConnected(1000))
    {
        emit started();
        emit status("OK: TCP-Socket connected to: " + host + ":" +
                     QString::number(port), StatusOk);
    }
    else
    {
        emit status("ERROR: TCP-Socket could not connect to: " + host +
                     ":" + QString::number(port), StatusError);
    }
}

void TcpSocket::stop()
{
    qDebug() << Q_FUNC_INFO;
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
    qDebug() << Q_FUNC_INFO<<socketError;

    switch (socketError)
    {
    case QAbstractSocket::RemoteHostClosedError:
        m_socketError = "The remote host closed the connection";
        break;

    case QAbstractSocket::HostNotFoundError:
        m_socketError = "The remote host is not found";
        break;

    case QAbstractSocket::ConnectionRefusedError:
        m_socketError = "Connection refused";
        break;

    default:
        break;
    }
}
