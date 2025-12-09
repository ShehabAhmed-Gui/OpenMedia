#include "foldermonitor.h"

FolderMonitor::FolderMonitor(QObject *parent)
    : QObject{parent}
{
    systemWatcher = new QFileSystemWatcher(this);

    connect(systemWatcher, &QFileSystemWatcher::fileChanged, this, &FolderMonitor::fileChanged);
    connect(systemWatcher, &QFileSystemWatcher::directoryChanged, this, &FolderMonitor::directoryChanged);
}

void FolderMonitor::addPath(const QString &absolutePath)
{
    if (absolutePath.isEmpty()) {
        qWarning() << "Expected non-empty path";
        return;
    }

    systemWatcher->addPath(absolutePath);
}

bool FolderMonitor::removePath(const QString &path)
{
    return systemWatcher->removePath(path);
}
