#include "frameprovider.h"

FrameProvider::FrameProvider(QSharedPointer<VideoManager> videoManager)
    :  QQuickImageProvider(QQuickImageProvider::Image)
    , m_videoManager(videoManager)
{
}

QImage FrameProvider::requestImage(const QString &id, QSize *size, const QSize &requestedSize)
{
    bool ok;
    quint64 timestamp = id.toInt(&ok);
    if (!ok) return QImage();

    QImage result = m_videoManager->getVideoFrame(timestamp);

    return result;
}
