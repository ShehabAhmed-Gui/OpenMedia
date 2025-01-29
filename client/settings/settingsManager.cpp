#include "settingsManager.h"

SettingsManager::SettingsManager(QObject *parent)
    : QObject{parent},
    m_settings(QSettings::NativeFormat, QSettings::UserScope, "OpenMedia", "OpenMedia"),
    m_settingsLocation(m_settings.fileName())
{}

SettingsManager::~SettingsManager()
{}

QStringList SettingsManager::getKeys(QString group)
{
    m_settings.beginGroup(group);
    auto keys = m_settings.allKeys();
    m_settings.endGroup();

    return keys;
}

void SettingsManager::removeKey(QString group, QString key)
{
    m_settings.beginGroup(group);
        m_settings.remove(key);
    m_settings.endGroup();
}

void SettingsManager::saveSettings(const QString &group, QString key, QVariant data) {
    m_settings.beginGroup(group);

    m_settings.setValue(key, data);

    m_settings.endGroup();

    m_settings.sync();
}

QVariant SettingsManager::getSetting(const QString &group, QString key, const QVariant &defaultValue) {
    m_settings.beginGroup(group);

    QVariant value = m_settings.value(key);

    m_settings.endGroup();

    if (key == "muted") return value.toBool();

    if (!value.isNull()) {
        return value;
    }
    return QVariant();
}
