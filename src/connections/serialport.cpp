#include "serialport.h"

#include <QSerialPortInfo>

SerialPort::SerialPort(QObject *parent) : NetConnection(parent),
    m_serialPort(new QSerialPort(this))
{
    connect(m_serialPort, &QSerialPort::readyRead, this, &SerialPort::onReadyRead);
    qDebug() << Q_FUNC_INFO;
}

SerialPort::~SerialPort()
{
    qDebug() << Q_FUNC_INFO;
}

void SerialPort::closePort()
{
    qDebug() << Q_FUNC_INFO;

    if(m_serialPort->isOpen())
        m_serialPort->close();
}

void SerialPort::start()
{
    qDebug() << Q_FUNC_INFO;

    closePort();

    const QString portName = settings().value(KEY_SERIAL_PORT).toString();
    const int baudRate = settings().value(KEY_BAUDRATE).toInt();

    m_serialPort->setPortName(portName);
    m_serialPort->setBaudRate(baudRate);
    m_serialPort->setDataBits(QSerialPort::Data8);
    m_serialPort->setParity(QSerialPort::NoParity);
    m_serialPort->setStopBits(QSerialPort::OneStop);
    m_serialPort->setFlowControl(QSerialPort::NoFlowControl);

    if (m_serialPort->open(QIODevice::ReadWrite))
    {
        emit started();
        emit status("OK: Serial-Port is open: " + portName, StatusOk);
    }
    else
    {
        emit status("ERROR: Serial-Port сould not open: " +
                     portName, StatusError);
    }
}

void SerialPort::stop()
{
    qDebug() << Q_FUNC_INFO;

    closePort();
    emit stopped();
    emit status("OK: Serial-Port stopped", StatusOk);
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

    for (const QSerialPortInfo &port: ports)
        result.append(port.portName());

    return result;
}

void SerialPort::onReadyRead()
{
    const QByteArray &data = m_serialPort->readAll();
    emit datagram(data, m_serialPort->portName(),
                  static_cast<quint16>(m_serialPort->baudRate() / 100));
}
