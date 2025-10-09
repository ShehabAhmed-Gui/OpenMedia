#ifndef VIDEOMANAGER_H
#define VIDEOMANAGER_H

#include "settings.h"
#include <QObject>
#include <QImage>
#include <QFile>

#include <QMutex>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
}

class VideoManager : public QObject
{
    Q_OBJECT
public:
    explicit VideoManager(QSharedPointer<Settings> settings,
                          QObject *parent = nullptr);

    ~VideoManager();

    Q_PROPERTY(Loop loopState READ loopState WRITE setLoopState NOTIFY loopStateChanged FINAL)

    enum Loop {
        Disabled = 0,
        Enabled = 1
    };

    Loop loopState() const;
    void setLoopState(Loop newLoopState);

    void setSourceVideo(const QString &path);

    QImage getVideoFrame(qint64 timestamp);

public slots:
    void extractVideoThumbnails(const QString &path);

signals:
    void loopStateChanged();
    void frameUpdated(int timestampMs);
    void extractingInProgress();
    void extractedVideoThumbnails();

private:
    void decode(AVCodecContext *dec_ctx, AVFrame *frame, AVPacket *pkt);

    Loop m_loopState = Disabled;

    QMap<quint64, QImage> m_thumbnails;

    QSharedPointer<Settings> m_settings;
};

#endif // VIDEOMANAGER_H
