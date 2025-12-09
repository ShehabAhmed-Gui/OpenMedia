#ifndef SETTINGSLOADER_H
#define SETTINGSLOADER_H

#include <QObject>
#include <QSharedDataPointer>
#include <QJsonObject>

#include "settings.h"

class SettingsLoader : public QObject
{
    Q_OBJECT
public:
    explicit SettingsLoader(const QSharedPointer<Settings> settings,
                            QObject *parent = nullptr);

    QJsonObject loadVolumeSettings();
    QJsonObject loadVideoSettings();
    QVector<QString> loadPlaylistSettings();

private:
    QSharedPointer<Settings> m_settings;
};

#endif // SETTINGSLOADER_H
