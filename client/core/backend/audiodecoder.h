#ifndef AUDIODECODER_H
#define AUDIODECODER_H

#include <QObject>
#include <QThread>
#include "decoder.h"
#include "demuxer.h"

class AudioDecoder : public Decoder
{
    Q_OBJECT
public:
    AudioDecoder();

    void stop();

public slots:
    void start(VideoState *state);

signals:
    void audioFrameReady(const QByteArray &pcm);

    // Decoder interface
public:
    void decode_video_frame(VideoState *state, QImage &image, AVPacket *pkt) override {
        qCritical() << "This function shouldn't be called from AudioDecoder!";
    }
    void decode_audio_frame(VideoState *state, QByteArray &buffer, AVPacket *pkt) override;

private:
    bool m_running = false;
};

#endif // AUDIODECODER_H
