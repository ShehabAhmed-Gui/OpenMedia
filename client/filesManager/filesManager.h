#ifndef APPMANAGER_H
#define APPMANAGER_H

#include <QObject>
#include <QFileDialog>
#include <QStringList>
#include <QDebug>
#include <QCoreApplication>
#include <QStandardPaths>
#include <QProcess>

#include "../settings/settingsManager.h"

class FilesManager : public QObject
{
    Q_OBJECT
public:
    explicit FilesManager(QObject *parent = nullptr);
    ~FilesManager();

    void addFile(QString path);

    Q_INVOKABLE QVector<QString> selectFiles();

    void setupDesktopFile();

signals:
    void videoPassedAsArg(QString path);

private:
    QFileDialog *dialog;

    QString m_defaultPath;
    QString supportedVids;
    QString m_selected;

    SettingsManager settings;
    QStringList selectedFiles;
};

#endif // APPMANAGER_H
