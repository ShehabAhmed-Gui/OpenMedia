#include "audiodecoder.h"

AudioDecoder::AudioDecoder()
{
}

void AudioDecoder::start(VideoState *state)
{
    m_running = true;
    int ret = 0;

    while(m_running) {
        while (state->paused && m_running) {
            QThread::msleep(10);
        }

        // Start decoding audio frames
        if (!state->audioq.isEmpty()) {
            AVPacket *ap = state->audioq.takeFirst();
            if (ap) {
                QByteArray pcm;
                decode_audio_frame(state, pcm, ap);
                emit audioFrameReady(pcm);
            }
        }
    }

    flush();
}

void AudioDecoder::stop()
{
    m_running = false;
}

void AudioDecoder::decode_audio_frame(VideoState *state, QByteArray &buffer, AVPacket *pkt)
{
    if (!pkt)
        return;

    int ret = avcodec_send_packet(m_ctx, pkt);
    if (ret < 0) {
        PlaybackLogger::printStringError(ret, "avcodec_send_packet failed:");
        return;
    }

    buffer.clear();
    while (true) {
        ret = avcodec_receive_frame(m_ctx, avframe);

        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
            break;
        }

        if (ret < 0) {
            PlaybackLogger::printStringError(ret, "avcodec_receive_frame failed:");
            return;
        }

        buffer.append(FrameConverter::rawtoPcm(avframe));
    }
}
