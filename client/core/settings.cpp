#include "settings.h"

Settings::Settings(QObject *parent)
    : QObject(parent)
{
    m_settings = new QSettings(this);
    m_settingsLocation = m_settings->fileName();
}

QStringList Settings::getKeys(QString group)
{
    m_settings->beginGroup(group);
    auto keys = m_settings->allKeys();
    m_settings->endGroup();

    return keys;
}

void Settings::removeKey(const QString &group, const QString &key)
{
    m_settings->beginGroup(group);
    m_settings->remove(key);
    m_settings->endGroup();
}

void Settings::removeGroup(const QString &group)
{
    m_settings->beginGroup(group);
    m_settings->remove("");
    m_settings->endGroup();
}

void Settings::saveSetting(const QString &group, QString key, QVariant data)
{
    m_settings->beginGroup(group);
    m_settings->setValue(key, data);
    m_settings->endGroup();

    m_settings->sync();
}

QVariant Settings::getSetting(const QString &group, QString key, const QVariant &defaultValue)
{
    m_settings->beginGroup(group);

    QVariant value = m_settings->value(key, defaultValue);

    m_settings->endGroup();

    if (!value.isNull()) {
        return value;
    }
    return QVariant();
}
