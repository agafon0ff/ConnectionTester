#ifndef SCRIPTOBJECTS_H
#define SCRIPTOBJECTS_H

#include <QObject>
#include <QScriptValue>
#include <QScriptEngine>

class JsConsole : public QObject
{
    Q_OBJECT
signals:
    void logMessage(const QString &text);
public slots:
    void log(const QString &text){emit logMessage(text);}
};

class JsJSON : public QObject
{
    Q_OBJECT
public:
    JsJSON(QScriptEngine *engine) : m_engine(engine){}

public slots:
    QScriptValue parse(const QString &text);
    QString stringify(const QScriptValue &value);

private:
    QScriptEngine *m_engine;
};


#endif // SCRIPTOBJECTS_H
