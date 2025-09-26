#ifndef VIDEOMANAGER_H
#define VIDEOMANAGER_H

#include <QObject>

class VideoManager : public QObject
{
    Q_OBJECT
public:
    explicit VideoManager(QObject *parent = nullptr);

signals:
};

#endif // VIDEOMANAGER_H
