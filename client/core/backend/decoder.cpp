#include "decoder.h"

#include <QDebug>
#include "playbacklogger.h"
#include "frameconverter.h"

Decoder::Decoder()
{
    avframe = av_frame_alloc();
}

Decoder::~Decoder()
{
    if (avframe)
        av_frame_free(&avframe);

    if (m_ctx)
        avcodec_free_context(&m_ctx);
}

bool Decoder::open(AVCodecParameters *codecpar)
{
    const AVCodec *decoder = avcodec_find_decoder(codecpar->codec_id);

    if (!decoder) {
        qCritical() << "Could not find a decoder";
        return false;
    }

    m_ctx = avcodec_alloc_context3(decoder);
    if (!m_ctx)
        return false;

    if (avcodec_parameters_to_context(m_ctx, codecpar) < 0)
        return false;

    if (avcodec_open2(m_ctx, decoder, nullptr) < 0)
        return false;

    qDebug() << "Found a decoder:" << decoder->name;
    return true;
}

void Decoder::decode_video_frame(VideoState *state, QImage &pic, AVPacket *pkt)
{
    if (!pkt)
        return;

    int ret = avcodec_send_packet(m_ctx, pkt);
    if (ret < 0) {
        PlaybackLogger::printStringError(ret, "avcodec_send_packet failed:");
        return;
    }

    while (true) {
        ret = avcodec_receive_frame(m_ctx, avframe);

        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
            break;
        }

        if (ret < 0) {
            PlaybackLogger::printStringError(ret, "avcodec_receive_frame failed:");
            return;
        }

        state->pts = avframe->pts;

        pic = FrameConverter::rawtoQImage(avframe);
    }
}

void Decoder::decode_audio_frame(QByteArray &buffer, AVPacket *pkt)
{
    if (!pkt)
        return;

    int ret = avcodec_send_packet(m_ctx, pkt);
    if (ret < 0) {
        PlaybackLogger::printStringError(ret, "avcodec_send_packet failed:");
        return;
    }

    while (true) {
        ret = avcodec_receive_frame(m_ctx, avframe);

        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
            break;
        }

        if (ret < 0) {
            PlaybackLogger::printStringError(ret, "avcodec_receive_frame failed:");
            return;
        }
        buffer = FrameConverter::rawtoPcm(avframe);
    }
}

void Decoder::flush()
{
    avcodec_send_packet(m_ctx, nullptr);
}

void Decoder::flushBuffers()
{
    avcodec_flush_buffers(m_ctx);
}

