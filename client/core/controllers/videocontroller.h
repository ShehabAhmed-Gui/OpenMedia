#ifndef VIDEOCONTROLLER_H
#define VIDEOCONTROLLER_H

#include <QObject>
#include <QSharedPointer>

#include "../videomanager.h"

class VideoController : public QObject
{
    Q_OBJECT
public:
    explicit VideoController(QSharedPointer<VideoManager> videoManager,
                             QObject *parent = nullptr);

    Q_PROPERTY(VideoManager::Loop loopState READ loopState WRITE setLoopState NOTIFY loopStateChanged FINAL)

    Q_INVOKABLE VideoManager::Loop loopState() const;
    Q_INVOKABLE void setLoopState(VideoManager::Loop newLoopState);


    Q_INVOKABLE void setSourceVideo(const QString &path);
    Q_INVOKABLE QImage readVideoFrameAt(quint64 timestampMs);

signals:
    void loopStateChanged();
    void frameUpdated(int timestamp);

private:
    QSharedPointer<VideoManager> m_videoManager;
    VideoManager::Loop m_loopState;
};

#endif // VIDEOCONTROLLER_H
