#ifndef APPMANAGER_H
#define APPMANAGER_H

#include <QObject>
#include <QFileDialog>
#include <QStringList>
#include <QDebug>
#include <QCoreApplication>
#include <QStandardPaths>
#include <QProcess>

#include "settings.h"

class FilesManager : public QObject
{
    Q_OBJECT
public:
    explicit FilesManager(const QSharedPointer<Settings> settings,
                          QObject *parent = nullptr);
    ~FilesManager();

    QVector<QString> selectFiles();

#ifdef Q_OS_LINUX
    void setupDesktopFile();
#endif

private:
    QFileDialog *dialog;
    QSharedPointer<Settings> m_settings;

    QString m_defaultPath;
    QString m_supportedFormats;
    QString m_selected;
    QStringList m_loadedFiles;
};

#endif // APPMANAGER_H
