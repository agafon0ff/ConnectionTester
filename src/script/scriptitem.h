#ifndef SCRIPTITEM_H
#define SCRIPTITEM_H

#include <QDebug>
#include <QWidget>
#include <QLabel>
#include <QToolButton>
#include <QScriptEngine>
#include <QScriptContext>
#include <QScriptValue>
#include <QScriptValueIterator>
#include "global.h"

class JsConsole;

class ScriptItem : public QObject
{
    Q_OBJECT

public:
    explicit ScriptItem(QObject *parent = Q_NULLPTR);
    ~ScriptItem();

private:

    QScriptEngine *m_engine;
    QString m_scriptName;
    QString m_scriptText;

    JsConsole *m_jsConsole;

    QScriptValue m_onScriptStartFunc;
    QScriptValue m_onScriptStopFunc;
    QScriptValue m_onReadTextFunc;
    QScriptValue m_onReadDataFunc;

signals:
    void started();
    void stopped();
    void status(const QString &text, int type);
    void datagram(const QByteArray &data, const QString &host, quint16 port);

public slots:
    void setName(const QString &name);
    QString name() const {return m_scriptName;}

    void setScript(const QString &text);
    QString scriptText() const {return m_scriptText;}

    void clearScript();

    void startScript();
    void stopScript();

    void setDatagram(const QByteArray &data, const QString &host, quint16 port);

public:
    static QByteArray arrayFromJsValue(const QScriptValue &jsArray);
    static QScriptValue jsValueFromArray(const QByteArray &ba, QScriptEngine *jsEngine);
};

class JsConsole : public QObject
{
    Q_OBJECT
signals:
    void logMessage(const QString &text);
public slots:
    void log(const QString &text){emit logMessage(text);}
};

#endif // SCRIPTITEM_H
