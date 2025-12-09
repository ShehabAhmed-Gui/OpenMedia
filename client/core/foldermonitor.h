#ifndef FOLDERMONITOR_H
#define FOLDERMONITOR_H

#include <QObject>
#include <QFileSystemWatcher>
#include <QStringList>
#include <QDebug>

class FolderMonitor : public QObject
{
    Q_OBJECT
public:
    explicit FolderMonitor(QObject *parent = nullptr);

    void start();
    void addPath(const QString &absolutePath);
    bool removePath(const QString &path);

signals:
    void directoryChanged(const QString &path);
    void fileChanged(const QString &path);

private:
    QStringList m_paths;
    QFileSystemWatcher *systemWatcher;
};

#endif // FOLDERMONITOR_H
