#include "scriptitem.h"
#include <QDebug>

QScriptValue onScriptStarted(QScriptContext *context, QScriptEngine *engine)
{
    Q_UNUSED(context);

    ScriptItem *script = dynamic_cast<ScriptItem*>(engine->parent());
    if (script) script->started();

    return QScriptValue();
}

QScriptValue onScriptStopped(QScriptContext *context, QScriptEngine *engine)
{
    Q_UNUSED(context);

    ScriptItem *script = dynamic_cast<ScriptItem*>(engine->parent());
    if (script) script->stopped();

    return QScriptValue();
}

QScriptValue onWriteText(QScriptContext *context, QScriptEngine *engine)
{
    Q_UNUSED(context);

    if(context->argumentCount() == 0)
        return QScriptValue();

    ScriptItem *script = dynamic_cast<ScriptItem*>(engine->parent());
    if (!script) return QScriptValue();


    if(context->argumentCount() == 3)
    {
        script->datagram(context->argument(0).toString().toUtf8(),
                         context->argument(1).toString(),
                         static_cast<quint16>(context->argument(2).toString().toInt()));
    }
    else script->datagram(context->argument(0).toString().toUtf8(), "", 1235);

    return QScriptValue();
}

QScriptValue onWriteData(QScriptContext *context, QScriptEngine *engine)
{
    Q_UNUSED(context);

    if(context->argumentCount() == 0)
        return QScriptValue();

    ScriptItem *script = dynamic_cast<ScriptItem*>(engine->parent());
    if (!script) return QScriptValue();

    QByteArray data = script->arrayFromJsValue(context->argument(0));

    if(context->argumentCount() == 3)
    {
        script->datagram(data, context->argument(1).toString(),
                         static_cast<quint16>(context->argument(2).toString().toInt()));
    }
    else script->datagram(data, "", 1235);

    return QScriptValue();
}

ScriptItem::ScriptItem(QObject *parent) :
    QObject(parent),
    m_engine(new QScriptEngine(this)),
    m_jsConsole(new JsConsole)
{
    connect(m_jsConsole, &JsConsole::logMessage,
            [=](const QString &text){emit status(text, StatusConsole);});
}

ScriptItem::~ScriptItem()
{
    delete m_jsConsole;
    delete m_engine;
}

void ScriptItem::setName(const QString &name)
{
    m_scriptName = name;
}

void ScriptItem::setScript(const QString &text)
{
    m_scriptText = text;

    QScriptValue jsConsoleObj =  m_engine->newQObject(m_jsConsole);
    m_engine->globalObject().setProperty("console", jsConsoleObj);

    QScriptValue funcStarted = m_engine->newFunction(onScriptStarted);
    m_engine->globalObject().setProperty("scriptStarted", funcStarted);

    QScriptValue funcStopped = m_engine->newFunction(onScriptStopped);
    m_engine->globalObject().setProperty("scriptStopped", funcStopped);

    QScriptValue funcWriteText = m_engine->newFunction(onWriteText);
    m_engine->globalObject().setProperty("writeText", funcWriteText);

    QScriptValue funcWriteData = m_engine->newFunction(onWriteData);
    m_engine->globalObject().setProperty("writeData", funcWriteData);

    QScriptContext* context = m_engine->pushContext();
    m_engine->evaluate(m_scriptText);

    m_onScriptStartFunc = context->activationObject().property("onScriptStart");
    m_onScriptStopFunc = context->activationObject().property("onScriptStop");
    m_onReadTextFunc = context->activationObject().property("onReadText");
    m_onReadDataFunc = context->activationObject().property("onReadData");
}

void ScriptItem::clearScript()
{
    m_engine->popContext();

    m_onScriptStartFunc = QScriptValue();
    m_onScriptStopFunc = QScriptValue();
    m_onReadTextFunc = QScriptValue();
    m_onReadDataFunc = QScriptValue();

    if(m_engine->isEvaluating())
        m_engine->abortEvaluation();
}

void ScriptItem::startScript()
{
    if(m_onScriptStartFunc.isFunction())
        m_onScriptStartFunc.call();
}

void ScriptItem::stopScript()
{
    if(m_onScriptStopFunc.isFunction())
        m_onScriptStopFunc.call();
}

void ScriptItem::setDatagram(const QByteArray &data, const QString &host, quint16 port)
{
    if (m_onReadTextFunc.isFunction())
    {
        QScriptValueList args;
        args << QString::fromUtf8(data) << host << static_cast<int>(port);
        m_onReadTextFunc.call(QScriptValue(), args);
    }

    if (m_onReadDataFunc.isFunction())
    {
        QScriptValueList args;
        args << jsValueFromArray(data, m_engine) << host << static_cast<int>(port);
        m_onReadDataFunc.call(QScriptValue(), args);
    }
}

QByteArray ScriptItem::arrayFromJsValue(const QScriptValue &jsArray)
{
    QByteArray ba;
    if(jsArray.isNull() || !jsArray.isArray())
        return ba;

    QScriptValueIterator it(jsArray);
    bool indexOk;
    while (it.hasNext())
    {
        it.next();
        it.name().toUInt(&indexOk);

        if(indexOk)
            ba.append(static_cast<quint8>(it.value().toUInt16()));
    }
    return ba;
}

QScriptValue ScriptItem::jsValueFromArray(const QByteArray &ba, QScriptEngine *jsEngine)
{
    QScriptValue array = jsEngine->newArray(ba.size());
    for(int i = 0; i < ba.size(); i++)
        array.setProperty(i, ba.at(i) & 0xFF);

    return array;
}
