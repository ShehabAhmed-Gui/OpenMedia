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

    void playFile(QString path);

    QVector<QString> selectFiles();

    void setupDesktopFile();

signals:
    void videoPassedAsArg(QString path);

private:
    QFileDialog *dialog;

    QSharedPointer<Settings> m_settings;

    QString m_defaultPath;
    QString supportedVids;
    QString m_selected;

    QStringList selectedFiles;
};

#endif // APPMANAGER_H
