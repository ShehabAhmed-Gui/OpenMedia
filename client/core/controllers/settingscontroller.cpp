#include "settingscontroller.h"

SettingsController::SettingsController(QSharedPointer<Settings> settings,
                                       QObject *parent)
    : QObject{parent}
    , m_settings(settings)
{}

QVariant SettingsController::getSetting(const QString &group, const QString &key, const QVariant &defaultValue)
{
    return m_settings->getSetting(group, key, defaultValue);
}

void SettingsController::saveSetting(const QString &group, QString key, QVariant data)
{
    m_settings->saveSetting(group, key, data);
}
