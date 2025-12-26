#ifndef VIDEODECODER_H
#define VIDEODECODER_H

#include <QObject>
#include <QThread>
#include "decoder.h"
#include "demuxer.h"

class VideoDecoder : public Decoder
{
    Q_OBJECT
public:
    VideoDecoder();

public:
    void stop();

    void decode_video_frame(VideoState *state, QImage &image, AVPacket *pkt) override;

public slots:
    void start(VideoState *state);

signals:
    void videoFrameReady(const QImage &image);

public:
    void decode_audio_frame(VideoState *state, QByteArray &buffer, AVPacket *pkt) override {
        qCritical() << "This function shouldn't be called from VideoDecoder!";
    }

private:
    bool m_running = false;
};

#endif // VIDEODECODER_H
