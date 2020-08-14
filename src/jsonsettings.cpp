#include "jsonsettings.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QVariant>

const QString FILE_CONFIG = "config.json";

JsonSettings::JsonSettings(QObject *parent) : QObject(parent)
{
    m_jsonSettings = loadJson(FILE_CONFIG);
}

void JsonSettings::setParameter(const QString &key, const QVariant &value)
{
    if(m_jsonSettings.contains(key))
        m_jsonSettings.remove(key);

    m_jsonSettings.insert(key,QJsonValue::fromVariant(value));
    saveJson(FILE_CONFIG,m_jsonSettings);
}

QVariant JsonSettings::getParameter(const QString &key, const QVariant &defaultValue)
{
    QVariant result = defaultValue;

    if(m_jsonSettings.contains(key))
        result = m_jsonSettings.value(key).toVariant();
    else
    {
        if(!defaultValue.isNull())
            setParameter(key, defaultValue);
    }

    return result;
}

void JsonSettings::setJsonObject(const QString &key, const QJsonObject jObject)
{
    if(m_jsonSettings.contains(key))
        m_jsonSettings.remove(key);

    m_jsonSettings.insert(key,jObject);
    saveJson(FILE_CONFIG,m_jsonSettings);
}

QJsonObject JsonSettings::getJsonObject(const QString &key, const QJsonObject &defaultObject)
{
    QJsonObject result = defaultObject;

    if(m_jsonSettings.contains(key))
        result = m_jsonSettings.value(key).toObject();
    else
    {
        if(!defaultObject.isEmpty())
            setJsonObject(key, defaultObject);
    }

    return result;
}

void JsonSettings::setParameters(const QMap<QString, QVariant> parameters)
{
    foreach(QString key, parameters.keys())
    {
        if(m_jsonSettings.contains(key))
            m_jsonSettings.remove(key);

        m_jsonSettings.insert(key,QJsonValue::fromVariant(parameters.value(key)));
    }

    saveJson(FILE_CONFIG,m_jsonSettings);
}

void JsonSettings::saveJson(const QString &fileName, const QJsonObject &jObject)
{
    QFileInfo fInfo(fileName);
    if(!fInfo.dir().exists())
    {
        QDir dir;
        dir.mkpath(fInfo.dir().path());
    }

    QFile file(fileName);
    if(file.open(QIODevice::WriteOnly))
    {
        file.write(QJsonDocument(jObject).toJson());
        file.close();
    }
}

QJsonObject JsonSettings::loadJson(const QString &fileName)
{
    QByteArray jsonData;
    QFile configFile(fileName);

    if (!configFile.open(QIODevice::ReadOnly))
        return QJsonObject();

    jsonData = configFile.readAll();
    configFile.close();

    QJsonParseError jsonError;
    QJsonObject jResult = QJsonDocument::fromJson(jsonData,&jsonError).object();

    if (jsonError.error != QJsonParseError::NoError)
        return QJsonObject();

    return jResult;
}
