#include "settingsloader.h"

SettingsLoader::SettingsLoader(const QSharedPointer<Settings> settings,
                               QObject *parent)
    : QObject{parent}
    , m_settings(settings)
{}

QJsonObject SettingsLoader::loadVolumeSettings()
{
    QJsonObject obj;

    const double volume = m_settings->getSetting("Audio", "volume").toDouble() / 100;
    const bool muted = m_settings->getSetting("Audio", "muted").toBool();

    obj.insert("volume", volume);
    obj.insert("muted", muted);
    return obj;
}

QJsonObject SettingsLoader::loadVideoSettings()
{
    QJsonObject obj;

    const QString path = m_settings->getSetting("Video", "video").toString();
    const QString mediaFolder = m_settings->getSetting("MediaPath", "mediaFolder").toString().remove("file://");
    const double position = m_settings->getSetting("Video", "position").toDouble() * 1000;
    const int loopState = m_settings->getSetting("Video", "loop").toInt();

    obj.insert("videoPath", path);
    obj.insert("mediaFolder", mediaFolder);
    obj.insert("position", position);
    obj.insert("loopState", loopState);
    return obj;
}

QVector<QString> SettingsLoader::loadPlaylistSettings()
{
    QVector<QString> mediaFiles;
    for (QString &item : m_settings->getKeys("Playlist")) {
        const QString &playlistItem = m_settings->getSetting("Playlist", item).toString();
        mediaFiles.append(playlistItem);
    }

    return mediaFiles;
}
