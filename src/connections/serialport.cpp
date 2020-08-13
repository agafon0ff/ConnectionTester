#include "serialport.h"

#include <QSerialPortInfo>

SerialPort::SerialPort(QObject *parent) : NetConnection(parent),
    m_serialPort(new QSerialPort(this))
{
    connect(m_serialPort, &QSerialPort::readyRead, this, &SerialPort::onReadyRead);
    qDebug()<<"*SerialPort";
}

SerialPort::~SerialPort()
{
    qDebug()<<"~SerialPort";
}

void SerialPort::closePort()
{
    qDebug()<<"SerialPort::closePort";

    if(m_serialPort->isOpen())
        m_serialPort->close();
}

void SerialPort::start()
{
    qDebug()<<"SerialPort::start";

    closePort();
    m_serialPort->setPortName(settings().host);
    m_serialPort->setBaudRate(QSerialPort::Baud9600);
    m_serialPort->setDataBits(QSerialPort::Data8);
    m_serialPort->setParity(QSerialPort::NoParity);
    m_serialPort->setStopBits(QSerialPort::OneStop);
    m_serialPort->setFlowControl(QSerialPort::NoFlowControl);

    if (m_serialPort->open(QIODevice::ReadWrite))
    {
        emit started();
        emit status("OK: Serial-Port is open: " + settings().host, StatusOk);
    }
    else
    {
        emit status("ERROR: Serial-Port сould not open: " +
                     settings().host, StatusError);
    }
}

void SerialPort::stop()
{
    qDebug()<<"SerialPort::stop";

    closePort();
    emit stopped();
    emit status("OK: UDP-Socket stopped", StatusOk);
}

void SerialPort::sendDatagram(const QByteArray &data, const QString &host, quint16 port)
{
    Q_UNUSED(host);
    Q_UNUSED(port);

    if(m_serialPort->isOpen())
    {
        m_serialPort->write(data);
        m_serialPort->waitForBytesWritten(300);
    }
}

QStringList SerialPort::availablePorts()
{
    const QList<QSerialPortInfo> &ports = QSerialPortInfo::availablePorts();
    QStringList result;

    foreach (const QSerialPortInfo &port, ports)
        result.append(port.portName());

    return result;
}

void SerialPort::onReadyRead()
{
    const QByteArray &data = m_serialPort->readAll();
    emit datagram(data, m_serialPort->portName(),
                  static_cast<quint16>(m_serialPort->baudRate() / 100));
}
