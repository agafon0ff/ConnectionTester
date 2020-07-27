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

NetSettingsStruct JsonSettings::settingsFromJson(const QJsonObject &jObject)
{
    NetSettingsStruct settings;
    settings.host = jObject.value(KEY_HOST).toString("127.0.0.1");
    settings.port = static_cast<quint16>(jObject.value(KEY_PORT).toInt(12345));

    settings.hostDst = jObject.value(KEY_HOST_DST).toString("127.0.0.1");
    settings.portDst = static_cast<quint16>(jObject.value(KEY_PORT_DST).toInt(12345));

    settings.autoStart = jObject.value(KEY_AUTO_START).toBool();
    settings.useMulticast = jObject.value(KEY_USE_MULTICAST).toBool();
    settings.showHex = jObject.value(KEY_SHOW_HEX).toBool();
    settings.showText = jObject.value(KEY_SHOW_TEXT).toBool(true);
    settings.addNewLine = jObject.value(KEY_ADD_NEW_LINE).toBool();
    settings.scriptsDir = jObject.value(KEY_SCRIPTS_DIR).toString();

    QStringList splitterH = jObject.value(KEY_SPLITTER_H).toString().split("|");
    QStringList splitterV = jObject.value(KEY_SPLITTER_V).toString().split("|");
    if (splitterH.size() == 2)settings.splitterH = TwoNumbers(splitterH.at(0).toInt(), splitterH.at(1).toInt());
    if (splitterV.size() == 2)settings.splitterV = TwoNumbers(splitterV.at(0).toInt(), splitterV.at(1).toInt());

    if(settings.splitterH.first == 0 && settings.splitterV.second == 0)
        settings.splitterH = TwoNumbers(800,100);

    if(settings.splitterV.first == 0 && settings.splitterV.second == 0)
        settings.splitterV = TwoNumbers(800,100);

    return settings;
}

QJsonObject JsonSettings::jsonFromSettings(const NetSettingsStruct &settings)
{
    QJsonObject jObject;
    jObject.insert(KEY_HOST, settings.host);
    jObject.insert(KEY_PORT, settings.port);

    jObject.insert(KEY_HOST_DST, settings.hostDst);
    jObject.insert(KEY_PORT_DST, settings.portDst);

    jObject.insert(KEY_AUTO_START, settings.autoStart);
    jObject.insert(KEY_USE_MULTICAST, settings.useMulticast);
    jObject.insert(KEY_SHOW_HEX, settings.showHex);
    jObject.insert(KEY_SHOW_TEXT, settings.showText);
    jObject.insert(KEY_ADD_NEW_LINE, settings.addNewLine);
    jObject.insert(KEY_SCRIPTS_DIR, settings.scriptsDir);

    jObject.insert(KEY_SPLITTER_H, QString::number(settings.splitterH.first) +
                   "|" + QString::number(settings.splitterH.second));

    jObject.insert(KEY_SPLITTER_V, QString::number(settings.splitterV.first) +
                   "|" + QString::number(settings.splitterV.second));
    return jObject;
}
