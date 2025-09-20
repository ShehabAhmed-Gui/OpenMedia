#ifndef SETTINGSCONTROLLER_H
#define SETTINGSCONTROLLER_H

#include <QObject>

#include "../settings.h"

class SettingsController : public QObject
{
    Q_OBJECT
public:
    explicit SettingsController(QSharedPointer<Settings> settings,
                                QObject *parent = nullptr);


    Q_INVOKABLE QVariant getSetting(const QString &group, const QString &key, const QVariant &defaultValue = QVariant());
    Q_INVOKABLE void saveSetting(const QString &group, QString key, QVariant data);

private:
    QSharedPointer<Settings> m_settings;
};

#endif // SETTINGSCONTROLLER_H
