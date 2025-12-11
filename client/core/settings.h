#ifndef SETTINGS_H
#define SETTINGS_H

#include <QObject>
#include <QSettings>
#include <QList>
#include <QDebug>

class Settings : public QObject
{
    Q_OBJECT

public:
    explicit Settings(QObject *parent = nullptr);

    QStringList getKeys(QString group);
    void removeKey(const QString &group, const QString &key);
    void removeGroup(const QString &group);
    void saveSetting(const QString &group, QString key, QVariant data);
    QVariant getSetting(const QString &group, QString key, const QVariant &defaultValue = QVariant());

private:
    QSettings *m_settings;
    QString m_settingsLocation;
};

#endif // SETTINGS_H
