#ifndef THUMBNAILSEXTRACTOR_H
#define THUMBNAILSEXTRACTOR_H

#include <QString>
#include <QObject>
#include <QMap>
#include <QImage>
#include <QDebug>
#include <qmutex.h>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
}

class ThumbnailsExtractor : public QObject
{
    Q_OBJECT
public:
    ThumbnailsExtractor(QObject *parent = nullptr);
    void extractVideoThumbnails(const QString &path);
    QImage getThumbnailAtTimestamp(qint64 timestamp);

signals:
    void extractingInProgress();
    void extractedVideoThumbnails();

private:
    QMap<quint64, QImage> m_thumbnails;
    QMutex m_mutex;
};

#endif // THUMBNAILSEXTRACTOR_H
