#ifndef SERIALPORT_H
#define SERIALPORT_H

#include "netconnection.h"
#include "global.h"

#include <QSerialPort>

class SerialPort : public NetConnection
{
    Q_OBJECT
public:
    explicit SerialPort(QObject *parent = Q_NULLPTR);
    ~SerialPort();

private:
    QSerialPort *m_serialPort;

    void closePort();
public slots:
    void start();
    void stop();

    void sendDatagram(const QByteArray &data, const QString &host, quint16 port);

    int type(){return ConnectionSerialPort;}
    QString getTypeString(){return "Serial Port";}

    static QStringList availablePorts();

private slots:
    void onReadyRead();
};

#endif // SERIALPORT_H
