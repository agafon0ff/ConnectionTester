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
#include "scriptobjects.h"

struct TimeoutStruct
{
    QScriptValue handler;
    QScriptValueList arguments;
    bool isTimeout = true;
};

class QTimerEvent;
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
    JsJSON *m_jsJson;

    QScriptValue m_onScriptStartFunc;
    QScriptValue m_onScriptStopFunc;
    QScriptValue m_onReadTextFunc;
    QScriptValue m_onReadDataFunc;

    QMap<int, TimeoutStruct> m_timeoutMap;
    void timerEvent(QTimerEvent *e);

signals:
    void started();
    void stopped();
    void status(const QString &text, int type);
    void datagram(const QByteArray &data, const QString &host, quint16 port);
    void clearText();

public slots:
    void setName(const QString &name);
    QString name() const {return m_scriptName;}

    void setScript(const QString &text);
    QString scriptText() const {return m_scriptText;}

    void clearScript();

    void startScript();
    void stopScript();

    void setDatagram(const QByteArray &data, const QString &host, quint16 port);
    int addTimer(QScriptContext *context, bool loop);
    void removeTimer(int id);

public:
    static QByteArray arrayFromJsValue(const QScriptValue &jsArray);
    static QScriptValue jsValueFromArray(const QByteArray &ba, QScriptEngine *jsEngine);
};

#endif // SCRIPTITEM_H
