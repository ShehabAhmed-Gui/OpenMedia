#include "videodecoder.h"
#include <QTimer>

VideoDecoder::VideoDecoder()
{
}

void VideoDecoder::start(VideoState *state)
{
    state->clock.start();
    m_running = true;
    int ret = 0;

    while(m_running) {
        while (state->paused && m_running) {
            QThread::msleep(10);
        }

        // Start decoding video frames
        if (!state->videoq.isEmpty()) {
            AVPacket *vp = state->videoq.takeFirst();
            if (vp) {
                QImage frame;
                decode_video_frame(state, frame, vp);

                emit videoFrameReady(frame);
            }
        }
    }

    flush();
    flushBuffers();
}

void VideoDecoder::stop()
{
    m_running = false;
}

void VideoDecoder::decode_video_frame(VideoState *state, QImage &image, AVPacket *pkt)
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

        qint64 pts_ms = av_rescale_q(
            state->pts,
            state->video_st->time_base,
            AVRational{1, 1000}
            );

        if (state->clock.alignment_offset == 0) {
            state->clock.alignment_offset =
                state->clock.clock.elapsed()
                - state->clock.pausedAccumulatedMs
                - pts_ms;
        }

        qint64 now_ms =
            state->clock.clock.elapsed()
            - state->clock.pausedAccumulatedMs
            - state->clock.alignment_offset;

        qint64 delay = pts_ms - now_ms;

        if (delay > 0) {
            QThread::msleep(double(delay));
        }

        state->position = pkt->pts;
        image = FrameConverter::rawtoQImage(avframe);
    }
}
