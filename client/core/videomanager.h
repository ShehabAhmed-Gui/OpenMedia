#ifndef VIDEOMANAGER_H
#define VIDEOMANAGER_H

#include "settings.h"
#include <QObject>
#include <QImage>

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

    QImage readVideoFrameAt(qint64 timestampMs);

signals:
    void loopStateChanged();
    void frameUpdated(int timestampMs);

private:
    QMutex m_decodeMutex;

    Loop m_loopState = Disabled;

    AVFrame* m_frame;
    AVFormatContext *fmtCtx;
    AVCodecContext *codecCtx;
    SwsContext *swsCtx;

    int videoStreamIndex;

    QSharedPointer<Settings> m_settings;
};

#endif // VIDEOMANAGER_H
