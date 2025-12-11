#ifndef APPMANAGER_H
#define APPMANAGER_H

#include <QObject>
#include <QFileDialog>
#include <QStringList>
#include <QDebug>
#include <QCoreApplication>
#include <QStandardPaths>
#include <QProcess>

#include "controllers/settingscontroller.h"
#include "foldermonitor.h"

class FilesManager : public QObject
{
    Q_OBJECT
public:
    explicit FilesManager(QSharedPointer<SettingsController> settingsController,
                          QSharedPointer<FolderMonitor> folderMonitor,
                          QObject *parent = nullptr);
    ~FilesManager();

    QVector<QString> selectFiles();

#ifdef Q_OS_LINUX
    void setupDesktopFile();
#endif

signals:
    void directoryChanged(const QString &path);
    void fileChanged(const QString &path);

private:
    QFileDialog *dialog;
    QSharedPointer<FolderMonitor> m_folderMonitor;
    QSharedPointer<SettingsController> m_settingsController;

    QString m_defaultPath = QStandardPaths::writableLocation(QStandardPaths::MusicLocation);
    QString m_supportedFormats;
    QString m_selected;
    QStringList m_loadedFiles;
};

#endif // APPMANAGER_H
