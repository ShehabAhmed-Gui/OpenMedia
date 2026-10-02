#include "videomanager.h"
#include <qthread.h>

#include "frameconverter.h"

VideoManager::VideoManager(QSharedPointer<Settings> settings,
                           QObject *parent)
    : QObject{parent}
    , m_settings(settings)
{
    m_loop = m_settings->getSetting("Video", "loop").toBool();
    m_thumbnailsExtractor = new ThumbnailsExtractor(this);

    connect(m_thumbnailsExtractor, &ThumbnailsExtractor::extractingInProgress, this, &VideoManager::extractingInProgress);
    connect(m_thumbnailsExtractor, &ThumbnailsExtractor::extractedVideoThumbnails, this, &VideoManager::extractedVideoThumbnails);
}

void VideoManager::openMediaFile(QString path)
{
#ifdef Q_OS_LINUX
    path = "file://" + path;
#endif

    emit playMediaFile(path);
}

bool VideoManager::loop() const
{
    return m_loop;
}

void VideoManager::setLoop(bool newLoop)
{
    if (m_loop == newLoop)
        return;

    m_loop = newLoop;
    emit loopChanged();

    m_settings->saveSetting("Video", "loop", newLoop);
}

QImage VideoManager::getThumbnailAtTimestamp(qint64 timestamp)
{
    return m_thumbnailsExtractor->getThumbnailAtTimestamp(timestamp);
}

void VideoManager::extractVideoThumbnails(const QString &path)
{
    m_thumbnailsExtractor->extractVideoThumbnails(path);
}
