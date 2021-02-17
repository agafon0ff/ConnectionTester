#include "udpsocket.h"
#include <QHostAddress>

UdpSocket::UdpSocket(QObject *parent) : NetConnection(parent),
    m_udpSocket(new QUdpSocket(this))
{
    connect(m_udpSocket,&QUdpSocket::readyRead, this, &UdpSocket::onSocketReadyRead);

    qDebug() << Q_FUNC_INFO;
}

UdpSocket::~UdpSocket()
{
    qDebug() << Q_FUNC_INFO;
}

void UdpSocket::closeSocket()
{
    if (m_udpSocket->state() == QAbstractSocket::BoundState)
        m_udpSocket->close();

    qDebug() << Q_FUNC_INFO;
}

void UdpSocket::start()
{
    qDebug() << Q_FUNC_INFO;
    closeSocket();

    const QString host = settings().value(KEY_HOST).toString();
    const quint16 port = static_cast<quint16>(settings().value(KEY_PORT).toInt());

    if (m_udpSocket->bind(QHostAddress::AnyIPv4, port,
                         QUdpSocket::ShareAddress | QUdpSocket::ReuseAddressHint))
    {
        if (settings().value(KEY_USE_MULTICAST).toBool())
        {
            m_udpSocket->setSocketOption(QAbstractSocket::MulticastTtlOption, 1);
            m_udpSocket->joinMulticastGroup(QHostAddress(host));
        }

        emit started();
        emit status("OK: UDP-Socket is listening on port: " +
                     QString::number(port), StatusOk);
    }
    else
    {
        emit status("ERROR: UDP-Socket сould not open port: " +
                     QString::number(port), StatusError);
    }
}

void UdpSocket::stop()
{
    qDebug() << Q_FUNC_INFO;
    closeSocket();

    emit stopped();
    emit status("OK: UDP-Socket stopped", StatusOk);
}

void UdpSocket::sendDatagram(const QByteArray &data, const QString &host, quint16 port)
{
//    qDebug()  << Q_FUNC_INFO << host << port << data;
    m_udpSocket->writeDatagram(data,QHostAddress(host),port);
}

void UdpSocket::onSocketReadyRead()
{
    while (m_udpSocket->hasPendingDatagrams())
    {
        QByteArray data;
        data.resize(m_udpSocket->pendingDatagramSize());
        QHostAddress senderHost;
        quint16 senderPort;

        m_udpSocket->readDatagram(data.data(), data.size(),
                                &senderHost, &senderPort);

//        qDebug()  << Q_FUNC_INFO << senderHost << senderPort << data;
        emit datagram(data, QHostAddress(senderHost.toIPv4Address()).toString(), senderPort);
    }
}
