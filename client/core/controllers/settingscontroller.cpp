#include "settingscontroller.h"

SettingsController::SettingsController(QSharedPointer<Settings> settings,
                                       QSharedPointer<SettingsLoader> settingsLoader,
                                       QObject *parent)
    : QObject{parent}
    , m_settings(settings)
    , m_settingsLoader(settingsLoader)
{
    QJsonObject volumeSettings = m_settingsLoader->loadVolumeSettings();
    muted = volumeSettings.value("muted").toBool();
    volumeLevel = volumeSettings.value("volume").toDouble();

    QJsonObject videoSettings = m_settingsLoader->loadVideoSettings();
    mediaPath = videoSettings.value("mediaFolder").toString();
    lastLoadedVideo = videoSettings.value("videoPath").toString();
    lastPosition = videoSettings.value("position").toDouble();
}

QVariant SettingsController::getSetting(const QString &group, const QString &key, const QVariant &defaultValue)
{
    return m_settings->getSetting(group, key, defaultValue);
}

void SettingsController::saveSetting(const QString &group, QString key, QVariant data)
{
    m_settings->saveSetting(group, key, data);
}

void SettingsController::removeGroup(const QString &group)
{
    m_settings->removeGroup(group);
}

bool SettingsController::isMuted() const
{
    return muted;
}

double SettingsController::getVolume() const
{
    return volumeLevel;
}

QString SettingsController::getMediaPath() const
{
    return mediaPath;
}

QString SettingsController::getLastLoadedVideo() const
{
    return lastLoadedVideo;
}

double SettingsController::getLastPosition() const
{
    return lastPosition;
}
