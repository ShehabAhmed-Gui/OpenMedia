#ifndef FILESCONTROLLER_H
#define FILESCONTROLLER_H

#include <QObject>

#include "../filesmanager.h"

class FilesController : public QObject
{
    Q_OBJECT
public:
    explicit FilesController(const QSharedPointer<FilesManager> filesManager,
                             QObject *parent = nullptr);

    Q_INVOKABLE QVector<QString> selectFiles();

private:
    QSharedPointer<FilesManager> m_filesManager;
};

#endif // FILESCONTROLLER_H
