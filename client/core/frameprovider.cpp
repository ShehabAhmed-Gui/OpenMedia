#include "frameprovider.h"

FrameProvider::FrameProvider(QSharedPointer<VideoManager> videoManager)
    :  QQuickImageProvider(QQuickImageProvider::Image)
    , m_videoManager(videoManager)
{
}

QImage FrameProvider::requestImage(const QString &id, QSize *size, const QSize &requestedSize)
{
    bool ok = false;
    const qint64 timestampMs = id.toLongLong(&ok);
    if (!ok)
        return {};

    QImage image = m_videoManager->getThumbnailAtTimestamp(timestampMs);
    if (!image.isNull() && requestedSize.isValid())
        image = image.scaled(requestedSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);

    if (size)
        *size = image.size();
    return image;
}
