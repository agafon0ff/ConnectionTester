#include "scriptobjects.h"
#include <QDebug>
#include <QScriptValueIterator>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

QScriptValue scriptFromJson(const QJsonValue &value, QScriptEngine *engine)
{
    if (value.isBool())
    {
        return QScriptValue(value.toBool());
    }
    else if (value.isString())
    {
        return QScriptValue(value.toString());
    }
    else if (value.isDouble())
    {
        return QScriptValue(value.toDouble());
    }
    else if (value.isNull())
    {
        return QScriptValue(QScriptValue::NullValue);
    }
    else if (value.isUndefined())
    {
        return QScriptValue(QScriptValue::UndefinedValue);
    }
    else if (value.isObject())
    {
        QJsonObject jObject = value.toObject();
        QScriptValue sObject = engine->newObject();

        for (const QString &key: jObject.keys())
        {
            QJsonValue jValue = jObject.value(key);
            sObject.setProperty(key, scriptFromJson(jValue, engine));
        }

        return sObject;
    }
    else if (value.isArray())
    {
        QJsonArray jArray = value.toArray();
        QScriptValue sObject = engine->newObject();

        for (int i=0; i<jArray.size(); ++i)
        {
            QJsonValue jValue = jArray[i];
            sObject.setProperty(i, scriptFromJson(jValue, engine));
        }

        return sObject;
    }

    return QScriptValue(QScriptValue::UndefinedValue);
}

QJsonValue jsonFromScript(const QScriptValue &value, QScriptEngine *engine)
{
    if (value.isBool())
    {
        return QJsonValue(value.toBool());
    }
    else if (value.isString())
    {
        return QJsonValue(value.toString());
    }
    else if (value.isNumber())
    {
        return QJsonValue(value.toNumber());
    }
    else if (value.isNull())
    {
        return QJsonValue(QJsonValue::Null);
    }
    else if (value.isUndefined())
    {
        return QJsonValue(QJsonValue::Undefined);
    }
    else if (value.isObject())
    {
        QScriptValue sObject = value.toObject();

        if(sObject.isArray())
        {
            QJsonArray jArray;
            int length = sObject.property("length").toInteger();

            for(int i = 0; i < length; ++i)
                jArray.append(jsonFromScript(sObject.property(i), engine));

            return jArray;
        }
        else
        {
            QJsonObject jObject;
            QScriptValueIterator it(sObject);
            while (it.hasNext())
            {
                it.next();
                jObject.insert(it.name(), jsonFromScript(it.value(), engine));
            }

            return jObject;
        }
    }

    return QJsonValue(QJsonValue::Undefined);
}

QScriptValue JsJSON::parse(const QString &text)
{
    QJsonParseError jsonError;
    QJsonDocument jDoc = QJsonDocument::fromJson(text.toUtf8(),&jsonError);
    if(jsonError.error != QJsonParseError::NoError)
        return QScriptValue(QScriptValue::UndefinedValue);

    if(jDoc.isObject())
        return scriptFromJson(jDoc.object(), m_engine);
    else if(jDoc.isArray())
        return scriptFromJson(jDoc.array(), m_engine);

    return QScriptValue(QScriptValue::UndefinedValue);
}

QString JsJSON::stringify(const QScriptValue &value)
{
    QByteArray jsonData = QJsonDocument(jsonFromScript(value,m_engine).toObject()).toJson(QJsonDocument::Compact);
    return QString::fromUtf8(jsonData);
}
