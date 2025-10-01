#include "videomanager.h"

VideoManager::VideoManager(QObject *parent)
    : QObject{parent}
{}

VideoManager::Loop VideoManager::loopState() const
{
    return m_loopState;
}

void VideoManager::setLoopState(Loop newLoopState)
{
    if (m_loopState == newLoopState)
        return;
    m_loopState = newLoopState;
    emit loopStateChanged();
}
