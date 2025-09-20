#include "filescontroller.h"

FilesController::FilesController(const QSharedPointer<FilesManager> filesManager,
                                 QObject *parent)
    : QObject{parent}
    , m_filesManager(filesManager)
{
}

QVector<QString> FilesController::selectFiles()
{
    return m_filesManager->selectFiles();
}
