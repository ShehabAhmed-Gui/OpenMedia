#include "videocontroller.h"

VideoController::VideoController(QSharedPointer<VideoManager> videoManager,
                                 QObject *parent)
    : QObject{parent}
    , m_videoManager(videoManager)
{
    connect(m_videoManager.get(), &VideoManager::frameUpdated, this, &VideoController::frameUpdated);
}

VideoManager::Loop VideoController::loopState() const
{
    return m_videoManager->loopState();
}

void VideoController::setLoopState(VideoManager::Loop newLoopState)
{
    if (loopState() == newLoopState)
        return;

    m_videoManager->setLoopState(newLoopState);
    emit loopStateChanged();
}

void VideoController::setSourceVideo(const QString &path)
{
    m_videoManager->setSourceVideo(path);
}

QImage VideoController::readVideoFrameAt(quint64 timestampMs)
{
    return m_videoManager->readVideoFrameAt(timestampMs);
}
