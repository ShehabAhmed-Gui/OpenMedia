#ifndef SETTINGSCONTROLLER_H
#define SETTINGSCONTROLLER_H

#include <QObject>

#include "../settings.h"
#include "../settingsloader.h"

class SettingsController : public QObject
{
    Q_OBJECT
public:
    explicit SettingsController(QSharedPointer<Settings> settings,
                                QSharedPointer<SettingsLoader> settingsLoader,
                                QObject *parent = nullptr);

    void removeGroup(const QString &group);

    Q_INVOKABLE QVariant getSetting(const QString &group, const QString &key, const QVariant &defaultValue = QVariant());
    Q_INVOKABLE void saveSetting(const QString &group, QString key, QVariant data);

    Q_INVOKABLE bool isMuted() const;
    Q_INVOKABLE double getVolume() const;

    Q_INVOKABLE QString getMediaPath() const;
    Q_INVOKABLE QString getLastLoadedVideo() const;
    Q_INVOKABLE double getLastPosition() const;

private:
    QSharedPointer<Settings> m_settings;
    QSharedPointer<SettingsLoader> m_settingsLoader;

    bool muted = false;
    double volumeLevel = 0.5;
    double lastPosition = 0;

    QString mediaPath = "";
    QString lastLoadedVideo = "";
};

#endif // SETTINGSCONTROLLER_H
