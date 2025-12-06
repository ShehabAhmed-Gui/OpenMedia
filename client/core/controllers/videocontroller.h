#ifndef VIDEOCONTROLLER_H
#define VIDEOCONTROLLER_H

#include <QObject>
#include <QSharedPointer>
#include <QThread>

#include "../videomanager.h"

class VideoController : public QObject
{
    Q_OBJECT
public:
    explicit VideoController(QSharedPointer<VideoManager> videoManager,
                             QObject *parent = nullptr);

    ~VideoController();

    Q_PROPERTY(VideoManager::Loop loopState READ loopState WRITE setLoopState NOTIFY loopStateChanged FINAL)

    Q_INVOKABLE VideoManager::Loop loopState() const;
    Q_INVOKABLE void setLoopState(VideoManager::Loop newLoopState);


    Q_INVOKABLE void extractVideoThumbnails(const QString &path);
    Q_INVOKABLE QImage getVideoFrame(quint64 timestamp);

signals:
    void loopStateChanged();
    void frameUpdated(int timestamp);
    void extractingInProgress();
    void extractedVideoThumbnails();

private:
    QSharedPointer<VideoManager> m_videoManager;
    VideoManager::Loop m_loopState;

    QThread *workerThread;
};

#endif // VIDEOCONTROLLER_H
