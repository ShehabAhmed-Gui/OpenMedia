#include "videocontroller.h"

VideoController::VideoController(QSharedPointer<VideoManager> videoManager,
                                 QObject *parent)
    : QObject{parent}
    , m_videoManager(videoManager)
{}

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
