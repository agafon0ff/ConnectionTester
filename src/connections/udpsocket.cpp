#include "udpsocket.h"
#include <QHostAddress>

UdpSocket::UdpSocket(QObject *parent) : NetConnection(parent),
    m_udpSocket(new QUdpSocket(this))
{
    connect(m_udpSocket,&QUdpSocket::readyRead, this, &UdpSocket::onSocketReadyRead);

    qDebug()<<"*UdpSocket";
}

UdpSocket::~UdpSocket()
{
    qDebug()<<"~UdpSocket";
}

void UdpSocket::closeSocket()
{
    if (m_udpSocket->state() == QAbstractSocket::BoundState)
        m_udpSocket->close();

    qDebug()<<"UdpSocket::closeSocket";
}

void UdpSocket::start()
{
    qDebug()<<"UdpSocket::start";
    closeSocket();

    if(m_udpSocket->bind(QHostAddress::AnyIPv4,settings().port,
                         QUdpSocket::ShareAddress | QUdpSocket::ReuseAddressHint))
    {
        if(settings().useMulticast)
        {
            m_udpSocket->setSocketOption(QAbstractSocket::MulticastTtlOption, 1);
            m_udpSocket->joinMulticastGroup(QHostAddress(settings().host));
        }

        emit started();
        emit status("OK: UDP-Socket is listening on port: " +
                     QString::number(settings().port), StatusOk);
    }
    else
    {
        emit status("ERROR: UDP-Socket сould not open port: " +
                     QString::number(settings().port), StatusError);
    }
}

void UdpSocket::stop()
{
    qDebug()<<"UdpSocket::stop";
    closeSocket();

    emit stopped();
    emit status("OK: UDP-Socket stopped", StatusOk);
}

void UdpSocket::sendDatagram(const QByteArray &data, const QString &host, quint16 port)
{
//    qDebug() << "UdpSocket::sendDatagram" << host << port << data;
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

//        qDebug() << "UdpSocket::onSocketReadyRead" << senderHost << senderPort << data;
        emit datagram(data, QHostAddress(senderHost.toIPv4Address()).toString(), senderPort);
    }
}