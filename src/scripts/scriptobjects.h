#ifndef SCRIPTOBJECTS_H
#define SCRIPTOBJECTS_H

#include <QObject>
#include <QScriptValue>
#include <QScriptEngine>

//!
//! \brief The JsConsole class
//!
class JsConsole : public QObject
{
    Q_OBJECT
signals:
    void logMessage(const QString &text);
    void clearText();
public slots:
    void log(const QString &text){emit logMessage(text);}
    void clear(){emit clearText();}
};


//!
//! \brief The JsJSON class
//!
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
