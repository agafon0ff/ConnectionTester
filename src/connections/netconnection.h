#ifndef NETCONNECTION_H
#define NETCONNECTION_H

#include <QObject>
#include <QMap>
#include "global.h"
#include "scriptitem.h"

class NetConnection : public QObject
{
    Q_OBJECT
public:
    explicit NetConnection(QObject *parent = Q_NULLPTR);
    virtual ~NetConnection();

private:
    NetSettingsStruct m_settings;
    QMap<QString, ScriptItem*> m_scriptsMap;

signals:
    void started();
    void stopped();
    void error(const QString &text);
    void status(const QString &text, int type);
    void datagram(const QByteArray &data, const QString &host, quint16 port);

public slots:
    virtual void start(){}
    virtual void stop(){}

    virtual void sendDatagram(const QByteArray &data, const QString &host, quint16 port)
    {Q_UNUSED(data); Q_UNUSED(host); Q_UNUSED(port);}

    virtual int type(){return -1;}
    virtual QString getTypeString(){return "";}

    void setSettings(const NetSettingsStruct &settings){m_settings = settings;}
    inline NetSettingsStruct settings(){return m_settings;}

    ScriptItem *addScript(const QString &name, const QString &text);
    QString scriptText(const QString &name);
    void removeScript(const QString &name);
};



#endif // NETCONNECTION_H
