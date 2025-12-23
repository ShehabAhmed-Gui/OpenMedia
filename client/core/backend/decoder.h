#ifndef DECODER_H
#define DECODER_H

#include <QImage>
#include <QList>
#include "../defs.h"

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
}

class Decoder
{
public:
    Decoder();
    ~Decoder();

    bool open(AVCodecParameters *codecpar);
    void decode_video_frame(VideoState *state, QImage &pic, AVPacket *pkt);
    void decode_audio_frame(QByteArray &buffer, AVPacket *pkt);
    void flush();
    void flushBuffers();

private:
    AVCodecContext *m_ctx = nullptr;
    AVFrame *avframe;
};

#endif // DECODER_H
