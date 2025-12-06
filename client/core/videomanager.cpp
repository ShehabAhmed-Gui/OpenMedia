#include "videomanager.h"

VideoManager::VideoManager(QSharedPointer<Settings> settings,
                           QObject *parent)
    : QObject{parent}
    , m_settings(settings)
{
    Loop savedState = static_cast<Loop>(m_settings->getSetting("Video", "loop").toInt());
    setLoopState(savedState);
    m_thumbnailsExtractor = new ThumbnailsExtractor(this);

    connect(m_thumbnailsExtractor, &ThumbnailsExtractor::extractingInProgress, this, &VideoManager::extractingInProgress);
    connect(m_thumbnailsExtractor, &ThumbnailsExtractor::extractedVideoThumbnails, this, &VideoManager::extractedVideoThumbnails);
}

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

    // Update loop state in Settings
    m_settings->saveSetting("Video", "loop", static_cast<int>(newLoopState));
}

QImage VideoManager::getVideoFrame(qint64 timestamp)
{
    return m_thumbnailsExtractor->getVideoFrame(timestamp);
}

void VideoManager::extractVideoThumbnails(const QString &path)
{
    m_thumbnailsExtractor->extractVideoThumbnails(path);
}
