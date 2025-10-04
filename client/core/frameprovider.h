#ifndef FRAMEPROVIDER_H
#define FRAMEPROVIDER_H

#include <QQuickImageProvider>
#include "videomanager.h"

class FrameProvider : public QQuickImageProvider
{
public:
    FrameProvider(QSharedPointer<VideoManager> videoManager);

    QImage requestImage(const QString &id, QSize *size, const QSize &requestedSize) override;

private:
    QSharedPointer<VideoManager> m_videoManager;
};

#endif // FRAMEPROVIDER_H
