#ifndef JSONSETTINGS_H
#define JSONSETTINGS_H

#include <QObject>
#include <QVariant>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include "global.h"

class JsonSettings : public QObject
{
    Q_OBJECT
public:
    explicit JsonSettings(QObject *parent = nullptr);

private:
    QJsonObject m_jsonSettings;

signals:

public slots:
    void setParameter(const QString &key, const QVariant &value);
    QVariant getParameter(const QString &key, const QVariant &defaultValue = QVariant());

    void setJsonObject(const QString &key, const QJsonObject jObject);
    QJsonObject getJsonObject(const QString &key, const QJsonObject &defaultObject = QJsonObject());

    void setParameters(const QMap<QString, QVariant> parameters);

    void saveJson(const QString &fileName, const QJsonObject &jObject);
    QJsonObject loadJson(const QString &fileName);

    NetSettingsStruct settingsFromJson(const QJsonObject &jObject);
    QJsonObject jsonFromSettings(const NetSettingsStruct &settings);
};

#endif // JSONSETTINGS_H
