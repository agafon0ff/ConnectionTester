#include "scriptitem.h"
#include <QDebug>
#include <QTimerEvent>

QScriptValue onScriptStarted(QScriptContext *context, QScriptEngine *engine)
{
    Q_UNUSED(context);

    ScriptItem *script = dynamic_cast<ScriptItem*>(engine->parent());
    if (script) emit script->started();

    return QScriptValue();
}

QScriptValue onScriptStopped(QScriptContext *context, QScriptEngine *engine)
{
    Q_UNUSED(context);

    ScriptItem *script = dynamic_cast<ScriptItem*>(engine->parent());
    if (script) emit script->stopped();

    return QScriptValue();
}

QScriptValue onWriteText(QScriptContext *context, QScriptEngine *engine)
{
    if(context->argumentCount() == 0)
        return QScriptValue();

    ScriptItem *script = dynamic_cast<ScriptItem*>(engine->parent());
    if (!script) return QScriptValue();


    if(context->argumentCount() == 3)
    {
        emit script->datagram(context->argument(0).toString().toUtf8(),
                         context->argument(1).toString(),
                         static_cast<quint16>(context->argument(2).toString().toInt()));
    }
    else emit script->datagram(context->argument(0).toString().toUtf8(), "", 0);

    return QScriptValue();
}

QScriptValue onWriteData(QScriptContext *context, QScriptEngine *engine)
{
    if(context->argumentCount() == 0)
        return QScriptValue();

    ScriptItem *script = dynamic_cast<ScriptItem*>(engine->parent());
    if (!script) return QScriptValue();

    QByteArray data = script->arrayFromJsValue(context->argument(0));

    if(context->argumentCount() == 3)
    {
        emit script->datagram(data, context->argument(1).toString(),
                         static_cast<quint16>(context->argument(2).toString().toInt()));
    }
    else emit script->datagram(data, "", 1235);

    return QScriptValue();
}

QScriptValue onSetTimeout(QScriptContext *context, QScriptEngine *engine)
{
    ScriptItem *script = dynamic_cast<ScriptItem*>(engine->parent());
    if (!script) return QScriptValue();
    return script->addTimer(context, false);
}

QScriptValue onSetInterval(QScriptContext *context, QScriptEngine *engine)
{
    ScriptItem *script = dynamic_cast<ScriptItem*>(engine->parent());
    if (!script) return QScriptValue();
    return script->addTimer(context, true);
}

QScriptValue onClearTimeout(QScriptContext *context, QScriptEngine *engine)
{
    ScriptItem *script = dynamic_cast<ScriptItem*>(engine->parent());
    if (!script) return QScriptValue();

    if(context->argumentCount() > 0)
        script->removeTimer(context->argument(0).toInt32());

    return QScriptValue();
}

QScriptValue onClearInterval(QScriptContext *context, QScriptEngine *engine)
{
    ScriptItem *script = dynamic_cast<ScriptItem*>(engine->parent());
    if (!script) return QScriptValue();

    if(context->argumentCount() > 0)
        script->removeTimer(context->argument(0).toInt32());

    return QScriptValue();
}

ScriptItem::ScriptItem(QObject *parent) :
    QObject(parent),
    m_engine(new QScriptEngine(this)),
    m_jsConsole(new JsConsole),
    m_jsJson(new JsJSON(m_engine))
{
    connect(m_jsConsole, &JsConsole::logMessage, this,
            [=](const QString &text){emit status(text, StatusType::StatusInput);});
    connect(m_jsConsole, &JsConsole::clearText, this, &ScriptItem::clearText);
}

ScriptItem::~ScriptItem()
{
    QMapIterator<int, TimeoutStruct> it(m_timeoutMap);
    while (it.hasNext())
    { it.next();  killTimer(it.key()); }

    m_timeoutMap.clear();

    delete m_jsJson;
    delete m_jsConsole;
    delete m_engine;
}

void ScriptItem::timerEvent(QTimerEvent *e)
{
    int id = e->timerId();

    if(m_timeoutMap.contains(id))
    {
        TimeoutStruct timeoutStruct = m_timeoutMap.value(id);
        timeoutStruct.handler.call(QScriptValue(), timeoutStruct.arguments);

        if(timeoutStruct.isTimeout)
        {
            m_timeoutMap.remove(id);
            killTimer(id);
        }
    }
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

    QScriptValue jsJsonObj =  m_engine->newQObject(m_jsJson);
    m_engine->globalObject().setProperty("JSON", jsJsonObj);

    QScriptValue funcStarted = m_engine->newFunction(onScriptStarted);
    QScriptValue funcStopped = m_engine->newFunction(onScriptStopped);
    QScriptValue funcWriteText = m_engine->newFunction(onWriteText);
    QScriptValue funcWriteData = m_engine->newFunction(onWriteData);
    QScriptValue funcSetTimeout = m_engine->newFunction(onSetTimeout);
    QScriptValue funcSetInterval = m_engine->newFunction(onSetInterval);
    QScriptValue funcClearTimeout = m_engine->newFunction(onClearTimeout);
    QScriptValue funcClearInterval = m_engine->newFunction(onClearInterval);

    m_engine->globalObject().setProperty("scriptStarted", funcStarted);
    m_engine->globalObject().setProperty("scriptStopped", funcStopped);
    m_engine->globalObject().setProperty("writeText", funcWriteText);
    m_engine->globalObject().setProperty("writeData", funcWriteData);
    m_engine->globalObject().setProperty("setTimeout", funcSetTimeout);
    m_engine->globalObject().setProperty("setInterval", funcSetInterval);
    m_engine->globalObject().setProperty("clearTimeout", funcClearTimeout);
    m_engine->globalObject().setProperty("clearInterval", funcClearInterval);

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

bool ScriptItem::isStartPosable()
{
    return m_onScriptStartFunc.isFunction();
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

int ScriptItem::addTimer(QScriptContext *context, bool loop)
{
    if (!context) return 0;

    TimeoutStruct timeoutStruct;
    timeoutStruct.isTimeout = !loop;
    timeoutStruct.handler = context->argument(0).toObject();
    int delay = static_cast<int>(context->argument(1).toInt32());

    for (int i=2; i<context->argumentCount(); ++i)
        timeoutStruct.arguments.append(context->argument(i));

    int result = startTimer(delay);
    m_timeoutMap.insert(result, timeoutStruct);
    return result;
}

void ScriptItem::removeTimer(int id)
{
    if (!m_timeoutMap.contains(id)) return;

    killTimer(id);
    m_timeoutMap.remove(id);
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
